## 0.7.0 (2026-07-29)

### Fix

- **resources**: fixes resource state test in case of spurious fails of weak comparision
- **tools**: fixes shader-compiler installation on windows
- fixes cyclonite package installation

### Feat

- windows platform support
- **tools**: make possible to compile shader compiler for windows

## 0.6.0 (2026-07-17)

### Fix

- **core**: fixes an issue with recursive lock on attempt to delete resource
- **vulkan**: fixes an issue when pipeline manager attempts to access invalid shader ref.
- **vulkan**: fixes wrong shader code size computation
- **core**: fixes issue when resourceId always refers to 0 index
- **gfx**: fixes device creation issue
- **tools**: fixes shader-compiler common enums
- **multithreading**: fixes missed include files

### Feat

- **gfx**: device limits now contains min uniform buffer offset alignment
- **cyclonite**: adds materials
- **resources**: adds default resource loader
- **multithreading**: when_all function now returns void future instead of vector of void results in case receives vector of void futures as argument.
- **core**: make possible to iterate over specified types of resources
- **shader-compiler**: adds new shader-compiler tool
- **tools**: adds tool to compile shader modules

### Refactor

- **cyclonite**: event class refactored

## gfx-first-mt-render (2025-10-28)

### Fix

- **core**: fixes an issue with implicitly deleted move assignment operator
- **multithreading**: fix an issue when executors waste CPU cycles even when all task queues are empty.
- fixes wrong platform options.

### Feat

- **multithreading**: adds multithreading utility functions when_all and when_any
- **multithreading**: adds the strandTask method that guarantees strictly sequential invocation of tasks submitted through.
- **multithreading**: splits general purpose and render tasks execution
- CRTP base application class is replaced with application concept
- **vulkan**: adds new type of exception for cases when exception is related to vulkan api.
- **core**: adds static hash table.
- **core**: adds functions to compute combined hash of combined keys.
- **tests**: adds tests for ring ranges
- **core**: adds BufferView class to present a piece of buffer as a view of necessary type.
- **vulkan**: adds device creation method implementation.
- **gfx**: gfx::Instance concept now requires instance to implement the method ::createDevice
- **gfx**: adds option to be able to select necessary graphics API.

### Refactor

- **sdl**: SDL initialization migrated into Root::init.
- platform option refactored

## 0.5.0 (2025-06-10)

### Feat

- migrate to Conan for dependency management
- **sdl**: sdl2 upgraded to sdl3
- target platform now is installation option.

### Refactor

- **sdl**: surface creation refactoring with respect to SDL3 changes.

### Fix

- **vulkan**: baseSurface now has explicit constructor
- **sdl**: available resolutions detection is refactored with respect to SDL3 api

## 0.4.0 (2023-01-05)

### Feat

- **compositor**: Compositor nodes are splitted into logic and gfx nodes. Compositor now tries to execute nodes in parallel if they do not depends on each other explicitly.
- **multithreading**: adds ability to propagate exceptions from workers to the main thread.
- **resources**: adds ability to load resource using a custom loader
- **compositor**: adds node config concept and traits

### Refactor

- **multithreading**: deprecated std::aligned_storage is replaced with byte array.

### Fix

- **multithreading**: fixes data races on attempt to submit a render task.
- fixes an issue when Clang compiler can not deduce config alias types.
- **vulkan**: fixes wrong android platform surface alias.
- **compositor**: fixes an issue when shader modules are got destroyed just after render pass creation.

## 0.3.0 (2022-11-07)

### Feat

- **multithreading**: new task manager based on WSD instead of boost::asio.

### Fix

- **vulkan**: fixes validation error on attempt to free empty command buffers.
- **resources**: fixes a crash on attempt to destroy resource manager.
- **resources**: fixes a crash on attempt to free dynamic buffer.

## 0.2.0 (2022-10-25)

### Feat

- **animations**: adds node based animation support.
- **resources**: resource now can have own post allocation handler

### Fix

- **systems**: fixes an issue with missed transform update
- **resources**: fixes an issue with ResourceList::Iterator
- **buffers**: fixes an issue with BufferView::Interator::operator-
- **buffers**: fixes BufferView<T>::Iterator comparison operators
- **buffers**: BufferView<T>::Iterator now is copyable
- **buffers**: fixes BufferView<T>::Iterator issues
- **buffers**: adds missed index operator for the BufferView<T>::Iterator

## 0.1.0 (2022-06-20)

### Feat

- **resources**: adds resource management

## 0.0.1 (2022-06-20)

## first-dummy-renderer (2022-05-12)

### Fix

- swap chain sync fix

## dummy-renderer (2020-01-11)
