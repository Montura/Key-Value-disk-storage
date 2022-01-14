#include <future>

#include "utils/boost_include.h"

namespace tests {
    class ThreadPool {
        basio::thread_pool pool;
    public:

        ThreadPool(int n_threads) : pool(n_threads) {}

        template <typename Func>
        void post(Func && f) {
            boost::asio::post(pool, std::forward<Func>(f));
        }

        void join() {
            pool.join();
        }

        template <typename Func>
        auto submit(Func && f) -> std::future<decltype(f())> {
            std::promise<decltype(f())> promise;
            auto future = promise.get_future();
            boost::asio::post(pool, [promise = std::move(promise), producer = std::forward<Func>(f)]() mutable {
                promise.set_value(producer());
            });
            return future;
        }
    };
}