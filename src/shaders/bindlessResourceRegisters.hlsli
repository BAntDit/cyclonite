
#ifndef BINDLESS_RESOURCE_REGISTERS_HLSLI
#define BINDLESS_RESOURCE_REGISTERS_HLSLI

#define GLOBAL_BINDLESS_SPACE 5             // must respect gfx::DescriptorSpace values
#define GLOBAL_CONSTANT_BUFFER_BINDING 0    // must respect gfx::BindingPoint::UNIFORM_BUFFER value
#define GLOBAL_STRUCTURED_BUFFER_BINDING 1  // must respect gfx::BindingPoint::UNIFORM_BUFFER value

// Register constants
#define RegisterConstants(Name, Body)                                         \
    struct Name##_t Body;                                                     \
    [[vk::binding(GLOBAL_CONSTANT_BUFFER_BINDING, GLOBAL_BINDLESS_SPACE)]]    \
    ConstantBuffer<Name##_t> Name[] : register(b0, space5)

// Register StructuredBuffers
#define RegisterStructuredBuffers(Name, Body)                                   \
    struct Name##_t Body;                                                       \
    [[vk::binding(GLOBAL_STRUCTURED_BUFFER_BINDING, GLOBAL_BINDLESS_SPACE)]]    \
    StructuredBuffer<Name##_t> Name[] : register(t1, space5)

#endif // BINDLESS_RESOURCE_REGISTERS_HLSLI

