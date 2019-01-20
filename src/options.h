//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_OPTIONS_H
#define CYCLONITE_OPTIONS_H

#include <string>

namespace cyclonite {
class Options
{
public:
    explicit Options(int argc = 0, const char* argv[] = {});

private:
    std::string config_;
};
}

#endif // CYCLONITE_OPTIONS_H
