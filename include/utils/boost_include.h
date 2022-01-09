#pragma once

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/thread/future.hpp>

#ifdef UNIT_TESTS
    #if USE_BOOST_PREBUILT_STATIC_LIBRARY
        #include <boost/test/unit_test.hpp>
    #else
        #include <boost/test/included/unit_test.hpp>
    #endif

    #include <boost/test/data/test_case.hpp>
    //#include <boost/range/iterator_range.hpp>
#endif

namespace m_boost {
    namespace bip = boost::interprocess;
    namespace basio = boost::asio;

    template <typename T>
    using BoostPackagedTask = boost::packaged_task<T>;
}

using namespace m_boost;