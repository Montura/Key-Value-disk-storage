#ifndef UNIT_TESTS
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <unordered_map>

template <typename T>
class CustomAllocator {
public:
    typedef T value_type;

    CustomAllocator() noexcept = default;

    template <typename U>
    explicit CustomAllocator (const CustomAllocator<U>&) noexcept {}

    T* allocate (std::size_t n) {
        void* p = std::malloc(n * sizeof(T));
        return static_cast<T*>(p);
    }

    void deallocate (T* p, std::size_t n) {
        free(p);
    }
};

using HashTableT = std::unordered_map < uint64_t, std::size_t,
                                        std::hash<uint64_t>,
                                        std::equal_to<>,
                                        CustomAllocator<std::pair<const uint64_t, std::size_t>>
                                        >;
HashTableT ma;
static uint64_t total_bytes_allocated = 0;
static uint64_t total_bytes_deallocated = 0;

void *operator new(std::size_t n) {
    void* p = std::malloc(n);
    if (!p)
        throw std::bad_alloc {};
    ma.insert(std::make_pair(reinterpret_cast<uint64_t>(p), n));
    total_bytes_allocated += n;
//    std::cout << "Alloc " << p << ", size is " << n << endl;
    return p;
}

void operator delete(void* mem) noexcept {
    auto pVoid = reinterpret_cast<uint64_t>(mem);
    std::size_t n = ma[pVoid];
    total_bytes_deallocated += n;
    ma.erase(pVoid);
//    std::cout << "Free " << mem << ", size is " << n << endl;
    free(mem);
}

void at_exit_handler() {
    std::cout << "Cleanup code after main()\n";
//    assert(ma.empty());
//    assert(total_bytes_allocated == total_bytes_deallocated);
    std::cout << "Total allocated " << total_bytes_allocated << " bytes\n";
    std::cout << "Total deallocated " << total_bytes_deallocated << " bytes\n";
}

//int main() {
//    int * a = new int[5];
//    delete[] a;
//    return 0;
//}
#endif // UNIT_TESTS