//
// Created by bantdit on 12/31/22.
//

#ifndef CYCLONITE_LOG_H
#define CYCLONITE_LOG_H

#include <boost/current_function.hpp>
#include <cstdio>
#include <sstream>

namespace cyclonite::log {
class ScopedLog
{
public:
    explicit ScopedLog(std::string_view scopeName);

    ~ScopedLog();

    template<typename Arg>
    auto operator<<(Arg arg) -> ScopedLog&
    {
        stream_ << arg;
        endl_ = false;

        return *this;
    }

    template<typename... Args>
    auto write(std::string_view message, Args&&... args) -> ScopedLog&
    {
        char buff[1024] = {};
        sprintf(buff, message.data(), std::forward<Args>(args)...);

        stream_ << buff;

        endl_ = false;
        return *this;
    }

    template<typename... Args>
    auto writeLine(std::string_view message, Args&&... args) -> ScopedLog&
    {
        if (!endl_) {
            stream_ << std::endl;
            endl_ = true;
        }

        char buff[1024] = {};
        sprintf(buff, message.data(), std::forward<Args>(args)...);

        stream_ << buff << std::endl;

        return *this;
    }

private:
    std::string scope_;
    std::ostringstream stream_;
    bool endl_;
};

#if !defined(NDEBUG)
// #define LOGGING_ENABLED
#endif

#if !defined(LOGGING_ENABLED)
#define SCOPED_LOG() int{};
#define LOG(LOGGER, ARG)
#define LOG_FMT(LOGGER, MSG, ...)
#define LOG_LINE(LOGGER, MSG, ...)
#else
#define SCOPED_LOG()                                                                                                   \
    log::ScopedLog { BOOST_CURRENT_FUNCTION }
#define LOG(LOGGER, ARG) LOGGER << ARG
#define LOG_FMT(LOGGER, MSG, ...) LOGGER.write(MSG, __VA_ARGS__)
#define LOG_LINE(LOGGER, MSG, ...) LOGGER.writeLine(MSG, __VA_ARGS__)
#endif

}

#endif // CYCLONITE_LOG_H
