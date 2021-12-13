#pragma once

#include "boost_include.h"

class MMapRegions final {
    std::vector<bio::mapped_file_source> mmap_regions;
public:
    explicit MMapRegions(const int size = 256): mmap_regions(size) {}

    template <typename It>
    void read_from_with_chunks(It it, const int chunk_size) {
        auto && reader = [&it, &chunk_size, offset = 0](bio::mapped_file_source &file) mutable {
            file.open(it, chunk_size, offset);
            offset += chunk_size;
        };
        do_work(std::move(reader));
    }

    template <typename DataTransformer, typename OutIt>
    void transform_data_and_write_to(DataTransformer && transformer, OutIt oit) {
        auto && writer = [&oit, tr = std::forward<DataTransformer>(transformer)](bio::mapped_file_source& file) {
            std::transform(file.data(), file.end(), oit, tr);
            file.close();
        };
        do_work(std::move(writer));
    }

    template <typename OutIt>
    void write_to(OutIt oit) {
        auto && writer = [&oit](bio::mapped_file_source& file) {
            std::copy(file.data(), file.end(), oit);
            file.close();
        };
        do_work(std::move(writer));
    }

    ~MMapRegions() {
        mmap_regions.clear();
    }

private:
    template <typename Func>
    void do_work(Func && func) {
        std::for_each(mmap_regions.begin(), mmap_regions.end(), std::forward<Func>(func));
    }
};