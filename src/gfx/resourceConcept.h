//
// Created by anton on 6/14/25.
//

#ifndef GFX_RESOURCE_CONCEPT_H
#define GFX_RESOURCE_CONCEPT_H

#include "common.h"
#include "surface.h"
#include <metrix/type_list.h>

namespace cyclonite::gfx {
using resource_type_list_t = metrix::type_list<type_traits::platform_implementation_t<gfx::Surface>>;
}

#endif // GFX_RESOURCE_CONCEPT_H
