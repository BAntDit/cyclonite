//
// Created by anton on 3/21/26.
//

#include "defaultResourceLoader.h"

namespace cyclonite::resources {
DefaultResourceLoader::DefaultResourceLoader(std::filesystem::path const& location)
  : loadInvoke_{ nullptr }
  , group_{ nullptr }
  , location_{ location }
{
}
}
