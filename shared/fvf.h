
#include <metrix/enum.h>
#include <cstdint>

namespace cyclonite::shared 
{
enum class VertexFormatFlags : uint64_t
{
    POSITION = 1ull << 0,    // float3 position
    NORMAL = 1ull << 1,      // float3 normal
    TANGENT = 1ull << 2,     // float3
    BINORMAL = 1ull << 3,    // float3 

    BONE_WEIGHTS = 1ull << 4, // bone weights as float32 vector4  
    BONE_INDICES = 1ull << 5, // bone indices as uint4

    COMPRESSED_SKINNING_DATA = 1ull << 6, // packed bone weights and indices as uint4 

    // The number of bits by which to shift an integer value that identifies the number of texture coordinates for a
    // vertex:
    TEX_COORD_COUNT_SHIFT = 6,

    TEX_COORD_COUNT_1 = 1ULL << 6,
    TEX_COORD_COUNT_2 = 1ULL << 7,
    TEX_COORD_COUNT_3 = 1ULL << 8,
    TEX_COORD_COUNT_4 = 1ULL << 9,
    TEX_COORD_COUNT_5 = 1ULL << 10,
    TEX_COORD_COUNT_6 = 1ULL << 11,
    TEX_COORD_COUNT_7 = 1ULL << 12,
    TEX_COORD_COUNT_8 = 1ULL << 13,
    TEX_COORD_COUNT_MASK = (TEX_COORD_COUNT_1 | TEX_COORD_COUNT_2 | TEX_COORD_COUNT_3 | TEX_COORD_COUNT_4 |
                            TEX_COORD_COUNT_5 | TEX_COORD_COUNT_6 | TEX_COORD_COUNT_7 | TEX_COORD_COUNT_8),

    // tex coord channel size encode:
    TEX_COORD_ZERO_CHANNEL_SHIFT = (1ULL << 14),
    TEX_COORD_01 = (1ULL << 14), // TEXCOORD0 channel 1
    TEX_COORD_02 = (1ULL << 15), // TEXCOORD0 channel 2
    TEX_COORD_03 = (1ULL << 16), // TEXCOORD0 channel 3
    TEX_COORD_04 = (1ULL << 17), // TEXCOORD0 channel 4
    TEX_COORD_11 = (1ULL << 18), // TEXCOORD1 channel 1
    TEX_COORD_12 = (1ULL << 19), // TEXCOORD1 channel 2
    TEX_COORD_13 = (1ULL << 20), // TEXCOORD1 channel 3
    TEX_COORD_14 = (1ULL << 21), // TEXCOORD1 channel 4
    TEX_COORD_21 = (1ULL << 22), // TEXCOORD2 channel 1 
    TEX_COORD_22 = (1ULL << 23), // TEXCOORD2 channel 2
    TEX_COORD_23 = (1ULL << 24), // TEXCOORD2 channel 3
    TEX_COORD_24 = (1ULL << 25), // TEXCOORD2 channel 4
    TEX_COORD_31 = (1ULL << 26), // TEXCOORD3 channel 1 
    TEX_COORD_32 = (1ULL << 27), // TEXCOORD3 channel 2
    TEX_COORD_33 = (1ULL << 28), // TEXCOORD3 channel 3
    TEX_COORD_34 = (1ULL << 29), // TEXCOORD3 channel 4
    TEX_COORD_41 = (1ULL << 20), // TEXCOORD4 channel 1 
    TEX_COORD_42 = (1ULL << 31), // TEXCOORD4 channel 2
    TEX_COORD_43 = (1ULL << 32), // TEXCOORD4 channel 3
    TEX_COORD_44 = (1ULL << 33), // TEXCOORD4 channel 4
    TEX_COORD_51 = (1ULL << 34), // TEXCOORD5 channel 1 
    TEX_COORD_52 = (1ULL << 35), // TEXCOORD5 channel 2
    TEX_COORD_53 = (1ULL << 36), // TEXCOORD5 channel 3
    TEX_COORD_54 = (1ULL << 37), // TEXCOORD5 channel 4
    TEX_COORD_61 = (1ULL << 38), // TEXCOORD6 channel 1 
    TEX_COORD_62 = (1ULL << 39), // TEXCOORD6 channel 2
    TEX_COORD_63 = (1ULL << 40), // TEXCOORD6 channel 3
    TEX_COORD_64 = (1ULL << 41), // TEXCOORD6 channel 4
    TEX_COORD_71 = (1ULL << 42), // TEXCOORD7 channel 1 
    TEX_COORD_72 = (1ULL << 43), // TEXCOORD7 channel 2
    TEX_COORD_73 = (1ULL << 44), // TEXCOORD7 channel 3
    TEX_COORD_74 = (1ULL << 45)  // TEXCOORD7 channel 4
};
using VertexFormatFlagBits = metrix::enum_bits<VertexFormatFlags>;
}
