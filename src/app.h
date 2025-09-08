//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_APP_H
#define CYCLONITE_APP_H

#include "commandLine.h"
#include <concepts>
#include <exception>
#include <iostream>
#include <type_traits>

namespace cyclonite {
template<typename T>
concept ApplicationConcept = requires(T t, CommadnLine const& commandLine)
{
    {
        t.init(commandLine)
    }
    ->std::same_as<T&>;

    {
        t.run()
    }
    ->std::same_as<T&>;

    {
        t.done()
    }
    ->std::same_as<void>;

    requires std::is_default_constructible_v<T>;
};

template<ApplicationConcept Application>
int letsGo(int argc, const char* argv[])
{
    try {
        Application{}.init(CommadnLine{ argc, argv }).run().done();
    } catch (std::exception const& e) {
        std::cout << "an exception has occurred: " << e.what() << std::endl;

        throw;
    }

    return 0;
}
}

#define CYCLONITE_APP(cls)                                                                                             \
    static_assert(cyclonite::ApplicationConcept<cls>, "Application must respect application concept");                 \
    int main(int argc, const char* argv[]) { return cyclonite::letsGo<cls>(argc, argv); }

#endif // CYCLONITE_APP_H
