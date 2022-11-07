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
