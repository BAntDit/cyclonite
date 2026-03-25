//
// Created by anton on 3/21/26.
//

#ifndef CYCLONITE_SHADER_H
#define CYCLONITE_SHADER_H

#include "core/resourceBase.h"
#include "resources/managedResource.h"

namespace cyclonite {
class Shader
  : public core::ResourceBase
  , public resources::ManagedResource<cyclonite::Shader>
{
public:
private:
};
}

#endif // CYCLONITE_SHADER_H
