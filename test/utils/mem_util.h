#if defined(MEM_CHECK)
//#include <iostream>
//#include <cstdlib>
//#include <memory>
//#include <unordered_map>
#include <limits>

//template <typename T>
//class CustomAllocator {
//public:
//    typedef T value_type;
//
//    CustomAllocator() noexcept = default;
//
//    template <typename U>
//    explicit CustomAllocator(const CustomAllocator<U>&) noexcept {}
//
//    T* allocate(std::size_t n) {
//        void* p = std::malloc(n * sizeof(T));
//        return static_cast<T*>(p);
//    }
//
//    void deallocate(T* p, std::size_t n) {
//        free(p);
//    }
//};
//
//using HashTableT = std::unordered_map < uint64_t, std::size_t,
//                                        std::hash<uint64_t>,
//                                        std::equal_to<>,
//                                        CustomAllocator<std::pair<const uint64_t, std::size_t>>
//                                        >;
//HashTableT ma;
static std::atomic<uint64_t> total_bytes_allocated = 0;
static std::atomic<uint64_t> total_bytes_deallocated = 0;

constexpr uint32_t max_size = std::numeric_limits<uint32_t>::max();
static uint16_t addresses[max_size];

uint32_t idx(uint64_t addr) {
    return addr % max_size;
}

void* operator new(std::size_t n) {
    void* pVoid = std::malloc(n);
    if (!pVoid)
        throw std::bad_alloc{};
    auto addr = reinterpret_cast<uint64_t>(pVoid);

    auto index = idx(addr);
    if (addresses[index] != 0)
        throw std::bad_alloc{};

    addresses[index] = n;
    total_bytes_allocated += n;
//    ma.emplace(addr, n);
//    std::cout << "Alloc " << p << ", size is " << n << std::endl;
    return pVoid;
}

void operator delete(void* mem) noexcept {
    auto addr = reinterpret_cast<uint64_t>(mem);
    auto index = idx(addr);

    auto n = addresses[index];
    total_bytes_deallocated += n;
    free(mem);
    addresses[index] = 0;

//    auto it = ma.find(addr);
//    if (it != ma.end()) {
//        total_bytes_deallocated += n;
//        ma.erase(addr);
//        free(mem);
//    }
//    std::cout << "Free " << mem << ", size is " << n << std::endl;
}
namespace tests {
    void at_exit_handler() {
        std::cout << "\tTotal allocated " << total_bytes_allocated << " bytes\n";
        std::cout << "\tTotal deallocated " << total_bytes_deallocated << " bytes\n";
        std::cout << "\tLost " << total_bytes_allocated - total_bytes_deallocated << " bytes\n" << std::endl;
//    assert(ma.empty());
//    assert(total_bytes_allocated == total_bytes_deallocated);
    }
}
#endif // MEM_CHECK