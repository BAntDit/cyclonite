//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_APP_H
#define CYCLONITE_APP_H

#include "defaultConfigs.h"
#include "options.h"
#include "updateStages.h"
#include <exception>
#include <iostream>

namespace cyclonite {
template<class Application>
class BaseApp
{
public:
    using platform_config_t = typename DefaultConfigs::platform_config_t;

    using ecs_config_t = typename DefaultConfigs::ecs_config_t;

    using config_t = Config<platform_config_t, ecs_config_t>;

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
