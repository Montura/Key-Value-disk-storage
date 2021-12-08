#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/spirit/include/qi.hpp>

namespace m_boost {
    namespace bio = boost::iostreams;
    namespace bip = boost::interprocess;
    namespace bc = boost::container;
    namespace bqi = boost::spirit::qi;

    template <typename T>
    using allocator = bip::allocator<T, bip::managed_mapped_file::segment_manager>;

    template <typename K, typename V>
    using map = bc::flat_map<
        K, V, std::less<K>,
        allocator<typename bc::flat_map<K, V>::value_type> >;

    template <typename K, typename V>
    using Map = map<K, V>;
}

using namespace m_boost;

// todo: investigate
//  https://stackoverflow.com/questions/28217301/using-boostiostreamsmapped-file-source-with-stdmultimap
void test_flat_map(const char* fname) {
    bip::managed_mapped_file msm(bip::open_or_create, "lookup.bin", 10ul << 20);
    Map<int, int>* lookup = msm.find_or_construct<Map<int, int>>("lookup")(msm.get_segment_manager());

    if (lookup->empty()) {
        // only read input file if not already loaded
        bio::mapped_file_source input(fname);
        auto f(input.data()), l(f + input.size());

        bool ok = bqi::phrase_parse(f, l,
                                   (bqi::auto_ >> bqi::auto_) % bqi::eol >> *bqi::eol,
                                    bqi::blank, *lookup);

        if (!ok || (f!=l))
            throw std::runtime_error("Error during parsing at position #" + std::to_string(f - input.data()));
    }
}

int main() {
   std::string_view in = "../mmap_in.data";
   std::string_view out = "../mmap_out.data";

    // mapped_file_source is read only
    // mapped_file_sink is write only
    // mapped_file is both read and write
    bio::mapped_file_source file;
    file.open(in.data());

    if (file.is_open()) {
        static_assert(sizeof(boost::uintmax_t) == sizeof(long long));
        auto fileSize = static_cast<long long>(fs::file_size(fs::path(in.data())));

        bio::mapped_file_params out_params;
        out_params.path = out.data();
        out_params.new_file_size = fileSize;
        out_params.flags = bio::mapped_file::mapmode::readwrite;

        bio::stream<bio::mapped_file_sink> out_mapped(out_params);

        std::copy(file.data(), file.data() + fileSize, std::ostream_iterator<char>(out_mapped));
        out_mapped.close();
    }
    file.close();


    return 0;
}