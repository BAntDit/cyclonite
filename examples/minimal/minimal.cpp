//
// Created by bantdit on 12/31/19.
//

#include "minimal.h"

namespace examples
{
Minimal::Minimal() : root_{ std::make_unique<cyclonite::Root<config_t>>() }
{}

auto Minimal::init(cyclonite::Options const& options) -> Minimal& {
    root_->init(options);
    return *this;
}

auto Minimal::run() -> Minimal& {
    return *this;
}

void Minimal::done() {}
}

CYCLONITE_APP(examples::Minimal)

