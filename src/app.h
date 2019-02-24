//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_APP_H
#define CYCLONITE_APP_H

#include "config.h"
#include "core/camera.h"
#include "core/transform.h"
#include "options.h"
#include "updateStages.h"
#include <easy-mp/enum.h>
#include <enttx/componentStorage.h>
#include <exception>
#include <iostream>

namespace cyclonite {
template<class Application>
class BaseApp
{
public:
    using config_t = Config<easy_mp::type_list<core::Transform, core::PerspectiveCamera, core::OrthographicCamera>,
                            easy_mp::type_list<enttx::ComponentStorage<64, 8, core::Transform>,
                                               enttx::ComponentStorage<8, 1, core::PerspectiveCamera>,
                                               enttx::ComponentStorage<8, 1, core::OrthographicCamera>>,
                            easy_mp::type_list<>,
                            easy_mp::value_cast(UpdateStage::COUNT)>;

    auto init(Options const& options) -> Application&;

    auto run() -> Application&;

    void done();
};

template<class Application>
auto BaseApp<Application>::init(Options const& options) -> Application&
{
    return static_cast<Application*>(this)->init(options);
}

template<class Application>
auto BaseApp<Application>::run() -> Application&
{
    return static_cast<Application*>(this)->run();
}

template<class Application>
void BaseApp<Application>::done()
{
    static_cast<Application*>(this)->done();
}

template<class Application>
int letsGo(int argc, const char* argv[])
{
    try {
        Options options(argc, argv);

        Application app;

        app.init(options).run().done();
    } catch (std::exception const& e) {
        std::cout << "an exception has occurred: " << e.what() << std::endl;

        throw;
    }

    return 0;
}
}

#define CYCLONITE_APP(cls)                                                                                             \
    int main(int argc, const char* argv[]) { return cyclonite::letsGo<cls>(argc, argv); }

#endif // CYCLONITE_APP_H
