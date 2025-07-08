//
// Created by bantdit on 11/2/20.
//

#ifndef CYCLONITE_VIEW_H
#define CYCLONITE_VIEW_H

#include "viewer.h"

namespace examples::viewer {
class Model;

class View
{
public:
    View() noexcept;

    void init(cyclonite::multithreading::TaskManager& taskManager);

    void draw();

    void dispose();
};
}

#endif // CYCLONITE_VIEW_H
