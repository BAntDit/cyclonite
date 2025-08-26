
#ifndef CYCLONITE_CORE_SPIN_LOCK
#define CYCLONITE_CORE_SPIN_LOCK

#include <atomic>

namespace cyclonite::core {
class SpinLock
{
public:
    void lock();

    void unlock();

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};
}

#endif // CYCLONITE_CORE_SPIN_LOCK
