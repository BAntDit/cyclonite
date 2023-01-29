
# CYCLONITE

Cyclonite is a graphics engine I mostly use for my own experiments.

![s1.png](./examples/gltf-viewer/screenshots/s1.png)

## Dependencies
- [GLM](https://github.com/g-truc/glm) - header only C++ mathematics library for graphics software.
- [SDL2](https://github.com/libsdl-org) - Simple DirectMedia Layer is a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware.
- [Enttx](https://github.com/BAntDit/enttx) - Compile-time and header-only Entity-Component-System library.
- [easy-mp](https://github.com/BAntDit/easy-mp) - Simple meta-programming library.
- [boost](https://www.boost.org/users/history/version_1_79_0.html) - The Boost C++ Libraries are a collection of modern libraries based on the C++ standard.
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) - The Vulkan SDK is a collection of essential tools used by developers to assist in development and debugging of Vulkan applications.

## Optional dependencies
- [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) - SPIRV-Cross is a tool designed for parsing and converting SPIR-V to other shader languages.

## Last update

Compositor nodes are split into logic and gfx nodes. 
Compositor now tries to execute nodes in parallel if they do not depend on each other explicitly.

[changelog](CHANGELOG.md)

## work in progress

Materials and batching.




