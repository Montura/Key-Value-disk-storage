#include <filesystem>
namespace fs = std::filesystem;

#include "boost_include.h"
#include "mmap_regions.h"
#include "btree.h"

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

/**
  mapped_file_source is read only
  mapped_file_sink is write only
  mapped_file is both read and write
     bio::mapped_file_params in_params;
     in_params.path = in.data();
     in_params.flags = bio::mapped_file::mapmode::readonly;
**/

// todo: move to tests
void check_file_size(const char* path, std::size_t const expected_size) {
    using ull = unsigned long long;
    static_assert(sizeof(boost::uintmax_t) == sizeof(ull));
    auto output_file_size = static_cast<ull>(fs::file_size(fs::path(path)));
    assert(output_file_size == expected_size);
}

void serialize_class(const char* path);
void serialize_stl(const char* path);

int main() {
    testBTree();

    serialize_class("../serialized_class");
    serialize_stl("../serialized_stl");

    constexpr std::size_t kB = 1024;
    constexpr std::size_t MB = 1024 * kB;
//    constexpr std::size_t GB = 1024 * MB;

// todo:check page size on Win platform
// https://docs.microsoft.com/en-us/windows-hardware/drivers/display/support-for-64kb-pages
#ifdef _MSC_VER
    constexpr std::size_t page_size = 64 * kB;
#else
    constexpr std::size_t page_size = 4 * kB;
#endif
    size_t size = bip::mapped_region::get_page_size();
    assert(page_size == size);

    std::string_view in = "../mmap_in.data";   // 1 MB
    std::string_view out = "../mmap_out.data"; // 1 MB

    constexpr std::size_t mapped_regions = MB / page_size;

    MMapRegions regions(mapped_regions);
    regions.read_from_with_chunks(in.data(), page_size);

    bio::mapped_file_params out_params;
    out_params.path = out.data();
    out_params.new_file_size = MB;
    out_params.flags = bio::mapped_file::mapmode::readwrite;

    bio::stream<bio::mapped_file_sink> out_mapped(out_params);

    regions.write_to(std::ostream_iterator<char>(out_mapped));
//    auto && w = [](char byte) -> char { return (byte != '\n') ? static_cast<char>(byte + 1) : byte; };
//    regions.transform_data_and_write_to(std::move(w), std::ostream_iterator<char>(out_mapped));

    check_file_size(out.data(), MB);

    // change file data wia mapped stream
    char *begin = out_mapped->data() + 2;
    *begin = 50;

    return 0;
}