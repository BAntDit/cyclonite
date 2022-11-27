//
// Created by bantdit on 11/20/22.
//

#ifndef CYCLONITE_NODEIDENTIFIER_H
#define CYCLONITE_NODEIDENTIFIER_H

#include "nodeTypeRegister.h"
#include <atomic>
#include <limits>

namespace cyclonite::compositor {
class NodeIdentifier
{
public:
    explicit NodeIdentifier(std::string_view name) noexcept;

    [[nodiscard]] auto id() const -> uint64_t { return id_; }

    [[nodiscard]] auto name() const -> std::string_view { return std::string_view{ name_.data(), name_.size() }; }

    [[nodiscard]] auto typeId() const -> uint64_t
    {
#if !defined(NDEBUG)
        return typeId_;
#else
        return std::numeric_limits<uint64_t>::max();
#endif
    }

    void _setTypeId([[maybe_unused]] uint64_t typeId)
    {
#if !defined(NDEBUG)
        typeId_ = typeId;
#endif
    }

    template<NodeConfig Config, typename NodeTypeId>
    [[nodiscard]] auto as(type_pair<node_t<Config>, NodeTypeId>) const -> node_t<Config> const&;

    template<NodeConfig Config, typename NodeTypeId>
    auto as(type_pair<node_t<Config>, NodeTypeId>) -> node_t<Config>&;

    static auto invalid() -> uint64_t { return std::numeric_limits<uint64_t>::max(); }

private:
    static std::atomic<uint64_t> _lasId;

    uint64_t id_;
    std::string name_;

#if !defined(NDEBUG)
    uint64_t typeId_;
#endif
};

template<NodeConfig Config, typename NodeTypeId>
auto NodeIdentifier::as(type_pair<node_t<Config>, NodeTypeId>) const -> node_t<Config> const&
{
#if !defined(NDEBUG)
    assert(typeId_ == NodeTypeId::value);
#endif
    return *(reinterpret_cast<node_t<Config> const*>(this));
}

template<NodeConfig Config, typename NodeTypeId>
auto NodeIdentifier::as(type_pair<node_t<Config>, NodeTypeId>) -> node_t<Config>&
{
#if !defined(NDEBUG)
    assert(typeId_ == NodeTypeId::value);
#endif
    return *(reinterpret_cast<node_t<Config>*>(this));
}
}

#endif // CYCLONITE_NODEIDENTIFIER_H
