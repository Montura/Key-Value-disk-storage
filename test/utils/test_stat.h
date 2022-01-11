#pragma once

#include <atomic>

namespace tests {
    class TestStat {
        int64_t N;
    public:
        int64_t total_exist = 0;
        int64_t total_not_exist = 0;
        int64_t total_found = 0;
        int64_t total_not_found = 0;
        int64_t total_removed = 0;
        int64_t total_after_remove = 0;
        int64_t total_after_reopen = 0;

        TestStat() : N(0) {}

        explicit TestStat(int64_t N) : N(N) {}

        bool all_exist() const {
            return total_exist == N;
        }

        bool any_does_not_exist() const {
            return total_not_exist == 0;
        }

        bool contains_all() const {
            return total_found == N;
        }

        bool any_not_found() const {
            return total_not_found == 0;
        }

        bool check_total_removed(int32_t expected) const {
            return total_removed == expected;
        }

        bool found_all_the_remaining() const {
            return total_after_remove == (N - total_removed);
        }
    };
}