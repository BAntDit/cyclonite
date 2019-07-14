//
// Created by bantdit on 7/14/19.
//

#include "base64Decode.h"
#include <array>

namespace cyclonite::loaders::gltf
{
    static std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    static inline bool isBase64(unsigned char c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::vector<std::byte> base64Decode(std::string const& src)
    {
        size_t inLength = src.size();
        size_t i = 0;
        size_t j = 0;
        size_t in_ = 0;

        std::array<unsigned char, 4> array4{0};
        std::array<unsigned char, 3> array3{0};

        std::vector<std::byte> result;

        while (inLength-- && (src[in_] != '=') && isBase64(static_cast<unsigned char>(src[in_]))) {
            array4[i++] = static_cast<unsigned char>(src[in_]);
            in_++;

            if (i == 4) {
                for (i = 0; i < 4; i++) {
                    array4[i] = static_cast<unsigned char>(base64Chars.find(array4[i]));
                }

                array3[0] = static_cast<unsigned char>((array4[0] << 2) + ((array4[1] & 0x30) >> 4));
                array3[1] = static_cast<unsigned char>(((array4[1] & 0xf) << 4) + ((array4[2] & 0x3c) >> 2));
                array3[2] = static_cast<unsigned char>(((array4[2] & 0x3) << 6) + array4[3]);

                for (i = 0; i < 3; i++) {
                    result.push_back(static_cast<std::byte>(array3[i]));
                }
                i = 0;
            }

            if (i != 0) {
                for (j = i; j < 4; j++) {
                    array4[j] = 0;
                }

                for (j = 0; j < 4; j++) {
                    array4[j] = static_cast<unsigned char>(base64Chars.find(array4[j]));
                }

                array3[0] = static_cast<unsigned char>((array4[0] << 2) + ((array4[1] & 0x30) >> 4));
                array3[1] = static_cast<unsigned char>(((array4[1] & 0xf) << 4) + ((array4[2] & 0x3c) >> 2));
                array3[2] = static_cast<unsigned char>(((array4[2] & 0x3) << 6) + array4[3]);

                for (j = 0; j < i - 1; j++) {
                    result.push_back(static_cast<std::byte>(array3[j]));
                }
            }
        }

        return result;
    }
}

