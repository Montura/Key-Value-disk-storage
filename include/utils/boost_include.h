#pragma once

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

#ifdef UNIT_TESTS
    #if USE_BOOST_PREBUILT_STATIC_LIBRARY
        #include <boost/test/unit_test.hpp>
    #else
        #include <boost/test/included/unit_test.hpp>
    #endif

#include <boost/range/iterator_range.hpp>
#include <boost/test/data/test_case.hpp>
#endif

namespace m_boost {
    namespace bip = boost::interprocess;
    namespace basio = boost::asio;
}

using namespace m_boost;