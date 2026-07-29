
# CYCLONITE

Cyclonite is a graphics engine I mostly use for my own experiments.

## Project installation

1. build and install shader-compiler package first: <br />
 ` conan install . -s compiler.cppstd=20 -s build_type=Release --build=missing --output-folder=cmake-build-release` <br />
 ` conan create . -s build_type=Release -s compiler.cppstd=20` <br />

2. install cyclonite dependencies: <br />
   `conan install . -s compiler.cppstd=20 -s build_type=Debug --build=missing --output-folder=cmake-build-debug -o platform=linux-x11`

## Last update

Compositor nodes are splitted into logic and gfx nodes. 
Compositor now tries to execute nodes in parallel if they do not depend on each other explicitly.

[changelog](CHANGELOG.md)

## work in progress

Materials and batching.




