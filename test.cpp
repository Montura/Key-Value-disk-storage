#if USE_BOOST_PREBUILT_STATIC_LIBRARY
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif

BOOST_AUTO_TEST_CASE(my_test)
{
    bool test_boost = true;

    BOOST_CHECK(test_boost);
}