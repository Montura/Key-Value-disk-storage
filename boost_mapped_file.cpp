#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
namespace bio = boost::iostreams;

int main() {
   std::string_view in = "../mmap_in.data";
   std::string_view out = "../mmap_out.data";

    boost::iostreams::mapped_file_source file;
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