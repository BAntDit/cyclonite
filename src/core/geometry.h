//
// Created by bantdit on 7/10/19.
//

#ifndef CYCLONITE_GEOMETRY_H
#define CYCLONITE_GEOMETRY_H

#include <cstdint>
#include <easy-mp/enum.h>
#include <easy-mp/type_list.h>
#include <variant>
#include <vector>

#include "attributeSemantic.h"
#include "primitiveType.h"

namespace cyclonite::core {
template<typename... Attributes>
class Geometry
{
public:
    using attribute_list_t = easy_mp::type_list<Attributes...>;

    using semantic_list_t = easy_mp::type_list<typename Attributes::semantic_t...>;

private:
    template<AttributeSemantic S>
    using get_attribute_semantic_t =
      std::integral_constant<std::underlying_type_t<AttributeSemantic>, easy_mp::value_cast(S)>;

    template<AttributeSemantic S>
    using get_attribute_index = typename semantic_list_t::template get_type_index<get_attribute_semantic_t<S>>;

public:
    template<AttributeSemantic S>
    auto getAttribute() -> decltype(std::get<get_attribute_index<S>::value>);

private:
    std::tuple<Attributes...> attributes_;

    PrimitiveType primitiveType_;

    uint64_t id_;
};
}

#endif // CYCLONITE_GEOMETRY_H
