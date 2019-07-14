//
// Created by bantdit on 7/14/19.
//

#ifndef CYCLONITE_BASE64DECODE_H
#define CYCLONITE_BASE64DECODE_H

#include <cstddef>
#include <string>
#include <vector>

namespace cyclonite::loaders::gltf
{
    std::vector<std::byte> base64Decode(std::string const& src);
}

#endif //CYCLONITE_BASE64DECODE_H
