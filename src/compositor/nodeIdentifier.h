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
    explicit NodeIdentifier(std::string_view name) noexcept;

    [[nodiscard]] auto id() const -> uint64_t { return id_; }

    [[nodiscard]] auto name() const -> std::string_view { return std::string_view{ name_.data(), name_.size() }; }

    static auto invalid() -> uint64_t { return std::numeric_limits<uint64_t>::max(); }

private:
    static std::atomic<uint64_t> _lasId;

    uint64_t id_;
    std::string name_;
};
}

#endif // CYCLONITE_NODEIDENTIFIER_H
