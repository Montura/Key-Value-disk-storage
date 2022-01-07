#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

namespace m_boost {
    namespace bio = boost::iostreams;
    namespace bip = boost::interprocess;
    namespace bc = boost::container;
    namespace bqi = boost::spirit::qi;
    namespace basio = boost::asio;

    template <typename T>
    using BoostPackagedTask = boost::packaged_task<T>;
}

using namespace m_boost;