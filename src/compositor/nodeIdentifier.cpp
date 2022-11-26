//
// Created by bantdit on 11/20/22.
//

#include "nodeIdentifier.h"

namespace cyclonite::compositor {
std::atomic<uint64_t> NodeIdentifier::_lasId{ 1 };

NodeIdentifier::NodeIdentifier()
  : id_{ _lasId.fetch_add(1, std::memory_order_relaxed) }
{}
}
