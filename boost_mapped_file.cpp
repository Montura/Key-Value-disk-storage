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

        if (!ok || (f != l))
            throw std::runtime_error("Error during parsing at position #" + std::to_string(f - input.data()));
    }
}

int main() {
    constexpr char END_OF_LINE = '\n';
    constexpr std::size_t kB = 1024;
    constexpr std::size_t page_size = 4 * kB;
    constexpr std::size_t MB = 1024 * kB;
//    constexpr std::size_t GB = 1024 * MB;

    static_assert(sizeof(boost::uintmax_t) == sizeof(long long));
    assert(page_size == bip::mapped_region::get_page_size());

    std::string_view in = "../mmap_in.data";   // 1 MB
    std::string_view out = "../mmap_out.data"; // 1 MB

    // mapped_file_source is read only
    // mapped_file_sink is write only
    // mapped_file is both read and write
//    bio::mapped_file_params in_params;
//    in_params.path = in.data();
//    in_params.flags = bio::mapped_file::mapmode::readonly;

    constexpr std::size_t mapped_regions = MB / page_size;
    std::vector<bio::mapped_file_source> mmapRegions(mapped_regions);
    std::for_each(
        mmapRegions.begin(),
        mmapRegions.end(),
        [&in, offset = 0](bio::mapped_file_source& file) mutable {
            file.open(in.data(), page_size, offset);
            offset += page_size;
        }
    );

    bio::mapped_file_params out_params;
    out_params.path = out.data();
    out_params.new_file_size = MB;
    out_params.flags = bio::mapped_file::mapmode::readwrite;

    bio::stream<bio::mapped_file_sink> out_mapped(out_params);
    std::for_each(
        mmapRegions.begin(),
        mmapRegions.end(),
        [&out_mapped](bio::mapped_file_source& file) {
            std::transform(file.data(), file.end(), std::ostream_iterator<char>(out_mapped),
                [](char byte) -> char { return (byte != END_OF_LINE) ? static_cast<char>(byte + 1) : byte; }
            );
            file.close();
        }
    );
    out_mapped.close();

    return 0;
}