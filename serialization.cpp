#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <unordered_map>

// https://www.boost.org/doc/libs/1_36_0/libs/serialization/doc/tutorial.html#nonintrusiveversion
class gps_position {
public:
    gps_position() : degrees(0), minutes(0), seconds(0) {};
    gps_position(int d, int m, float s) : degrees(d), minutes(m), seconds(s) {}
    int degrees;
    int minutes;
    float seconds;

    // https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
    template <typename FloatingPoint>
    bool combinedToleranceCompare(FloatingPoint x, FloatingPoint y) const {
        auto maxXYOne = std::max<FloatingPoint>({ 1.0, std::fabs(x), std::fabs(y) });
        return std::fabs(x - y) <= std::numeric_limits<FloatingPoint>::epsilon() * maxXYOne;
    }

    bool operator==(const gps_position& other) const {
        return  (degrees == other.degrees) &&
                (minutes == other.degrees) &&
                (combinedToleranceCompare(seconds, other.seconds));
    }
};

namespace boost::serialization {
    template<class Archive>
    void serialize(Archive &ar, gps_position &g, const unsigned int version) {
        ar & g.degrees;
        ar & g.minutes;
        ar & g.seconds;
    }
} // namespace boost

void serialize_class(const char* path) {
    // create and open a character archive for output
    std::ofstream ofs(path);

    // create class instance
    const gps_position instance(35, 59, 24.567f);

    // save data to archive
    {
        boost::archive::text_oarchive oa(ofs);
        // write class instance to archive
        oa << instance;
        // archive and stream closed when destructors are called
    }

    // ... some time later restore the class instance to its orginal state
    gps_position new_instance;
    {
        // create and open an archive for input
        std::ifstream ifs(path);
        boost::archive::text_iarchive ia(ifs);
        // read class state from archive
        ia >> new_instance;
        // archive and stream closed when destructors are called
    }
    assert(instance == new_instance);
}

// https://www.boost.org/doc/libs/1_36_0/libs/serialization/doc/tutorial.html#stl
void serialize_stl(const char* path) {
    // create and open a character archive for output
    std::ofstream ofs(path);

    // create class instance
    std::unordered_map<int, std::string> map;
    for (int i = 0; i < 1000; ++i) {
        map[i] = std::to_string(i);
    }
    // save data to archive
    {
        boost::archive::text_oarchive oa(ofs);
        // write class instance to archive
        oa << map;
        // archive and stream closed when destructors are called
    }

    // ... some time later restore the class instance to its orginal state
    std::unordered_map<int, std::string> new_map;
    {
        // create and open an archive for input
        std::ifstream ifs(path);
        boost::archive::text_iarchive ia(ifs);
        // read class state from archive
        ia >> new_map;
        // archive and stream closed when destructors are called
    }

    assert(map == new_map);
}