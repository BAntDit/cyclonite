//
// Created by anton on 2/5/23.
//

#ifndef CYCLONITE_MATERIAL_H
#define CYCLONITE_MATERIAL_H

#include "resources/resource.h"
#include "typedefs.h"
#include <array>
#include <bitset>

namespace cyclonite {
class Material : public resources::Resource
{
public:
public:
    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

private:
    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return Material::tag; }
    static auto type_tag() -> ResourceTag& { return Material::tag; }

private:
    std::string name_;
};
}

#endif // CYCLONITE_MATERIAL_H
