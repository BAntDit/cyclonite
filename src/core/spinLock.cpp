
#include "spinLock.h"
#include <thread>

namespace cyclonite::core {
void SpinLock::lock()
{
    while (flag_.test_and_set(std::memory_order_acquire)) {
        flag_.wait(true, std::memory_order_relaxed);
    }
}

void SpinLock::unlock()
{
    flag_.clear(std::memory_order_release);
    flag_.notify_one();
}

}
