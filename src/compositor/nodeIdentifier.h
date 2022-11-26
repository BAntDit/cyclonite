//
// Created by bantdit on 11/20/22.
//

#ifndef CYCLONITE_NODEIDENTIFIER_H
#define CYCLONITE_NODEIDENTIFIER_H

#include <atomic>
#include <limits>

namespace cyclonite::compositor {
class NodeIdentifier
{
public:
    NodeIdentifier() noexcept;

    [[nodiscard]] auto id() const -> uint64_t { return id_; }

    static auto invalid() -> uint64_t { return std::numeric_limits<uint64_t>::max(); }

private:
    static std::atomic<uint64_t> _lasId;

    uint64_t id_;
};
}

#endif // CYCLONITE_NODEIDENTIFIER_H
