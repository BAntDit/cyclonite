//
// Created by bantdit on 12/28/19.
//

#ifndef CYCLONITE_MINIMAL_H
#define CYCLONITE_MINIMAL_H

#include "cyclonite.h"

namespace examples
{
class Minimal final: public cyclonite::BaseApp<Minimal>, public cyclonite::EventReceivable
{
public:
    Minimal();

    auto init(cyclonite::Options const& options) -> Minimal&;

    auto run() -> Minimal&;

    void done();

private:
    std::unique_ptr<cyclonite::Root<config_t>> root_;
};
}

#endif //CYCLONITE_MINIMAL_H
