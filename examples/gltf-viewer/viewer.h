//
// Created by bantdit on 8/2/20.
//

#ifndef CYCLONITE_VIEWER_H
#define CYCLONITE_VIEWER_H

#include <cyclonite.h>

namespace examples::viewer {
class Model;

class Viewer final : public cyclonite::BaseApp<Viewer>
{
public:
    Viewer();

    auto init(cyclonite::Options options) -> Viewer&;

    auto run() -> Viewer&;

    void done();

private:
    std::unique_ptr<cyclonite::Root> root_;
    std::unique_ptr<Model> model_;
};
}

#endif // CYCLONITE_VIEWER_H
