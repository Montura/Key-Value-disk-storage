#pragma once

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