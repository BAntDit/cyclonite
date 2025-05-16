//
// Created by bantdit on 9/27/19.
//

#ifndef CYCLONITE_ERROR_H
#define CYCLONITE_ERROR_H

#include <stdexcept>

namespace cyclonite {
struct Error : std::runtime_error
{
    using std::runtime_error::runtime_error;

    explicit Error(std::string const& what)
      : std::runtime_error(what)
    {
    }
};
}

#endif // CYCLONITE_ERROR_H
