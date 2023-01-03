//
// Created by bantdit on 1/1/23.
//

#include "log.h"
#include <chrono>
#include <iostream>

namespace cyclonite::log {
ScopedLog::ScopedLog(std::string_view scopeName)
  : scope_{ scopeName }
  , stream_{}
  , endl_{ false }
{
    auto t = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(t);
    stream_ << "start: " << std::ctime(&tt) << ": " << scope_ << ": ";
}

ScopedLog::~ScopedLog()
{
    if (!endl_) {
        stream_ << std::endl;
        endl_ = true;
    }

    std::cerr << stream_.str() << std::endl;
    std::cerr << "end: " << scope_ << std::endl;
}
}
