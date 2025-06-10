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
