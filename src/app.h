//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_APP_H
#define CYCLONITE_APP_H

#include "options.h"
#include <exception>
#include <iostream>

namespace cyclonite {
template<class Application>
class BaseApp
{
public:
    auto init(Options options) -> Application&;

    auto run() -> Application&;

    void done();
};

template<class Application>
auto BaseApp<Application>::init(Options options) -> Application&
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
        Application{}.init(Options{ argc, argv }).run().done();
    } catch (std::exception const& e) {
        std::cout << "an exception has occurred: " << e.what() << std::endl;

        throw;
    }

    return 0;
}
}

#define CYCLONITE_APP(cls)                                                                                             \
    int main(int argc, const char* argv[])                                                                             \
    {                                                                                                                  \
        return cyclonite::letsGo<cls>(argc, argv);                                                                     \
    }

#endif // CYCLONITE_APP_H
