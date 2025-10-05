//
// Created by anton on 6/19/25.
//

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "hash.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace cyclonite::core {
namespace internal {
template<typename Key>
concept HashTableKeyConcept = std::is_default_constructible_v<Key>;

template<typename Data>
concept HashTableDataConcept =
  (std::is_default_constructible_v<Data> && std::is_copy_constructible_v<Data> && std::is_copy_assignable_v<Data>);

template<HashTableDataConcept DataType, size_t TableSize, HashTableKeyConcept... Key>
class HashTableData
{
public:
    using key_type = std::tuple<Key...>;

    using value_type = std::pair<std::add_const_t<key_type>, DataType>;

    static constexpr auto invalid_hash_entry_v = std::numeric_limits<size_t>::max();

    HashTableData() noexcept;

    void clear();

    [[nodiscard]] auto data() const -> std::array<value_type, TableSize> const& { return data_; }
    [[nodiscard]] auto data() -> std::array<value_type, TableSize>& { return data_; }

    [[nodiscard]] auto size() const -> size_t { return actualSize_; }
    [[nodiscard]] auto capacity() const -> size_t { return TableSize; }

    [[nodiscard]] auto originalHashEntries() const -> std::array<size_t, TableSize> const&
    {
        return originalHashEntry_;
    }
    [[nodiscard]] auto originalHashEntries() -> std::array<size_t, TableSize>& { return originalHashEntry_; }

    [[nodiscard]] auto hashEntryCounts() const -> std::array<uint32_t, TableSize> const& { return hashEntryCount_; }
    [[nodiscard]] auto hashEntryCounts() -> std::array<uint32_t, TableSize>& { return hashEntryCount_; }

    void increaseEntryCount() { actualSize_++; }
    void decreaseEntryCount() { actualSize_--; }

    void destroyEntryData(size_t index);

private:
    std::array<value_type, TableSize> data_;
    std::array<uint32_t, TableSize> hashEntryCount_;
    std::array<size_t, TableSize> originalHashEntry_;
    size_t actualSize_;
};

template<HashTableDataConcept DataType, size_t TableSize, HashTableKeyConcept... Key>
HashTableData<DataType, TableSize, Key...>::HashTableData() noexcept
  : data_{}
  , hashEntryCount_{}
  , originalHashEntry_{}
  , actualSize_{ 0 }
{
    clear();
}

template<HashTableDataConcept DataType, size_t TableSize, HashTableKeyConcept... Key>
void HashTableData<DataType, TableSize, Key...>::clear()
{
    for (auto i = size_t{ 0 }; i < TableSize; i++) {
        destroyEntryData(i);
    }
    hashEntryCount_.fill(0);
    actualSize_ = 0;
}

template<HashTableDataConcept DataType, size_t TableSize, HashTableKeyConcept... Key>
void HashTableData<DataType, TableSize, Key...>::destroyEntryData(size_t index)
{
    assert(index < TableSize);

    auto destroy_keys_f = []<size_t... I>(std::index_sequence<I...>, auto&& tuple) -> void {
        auto destroy_key_f = [](auto&& key) -> void {
            if constexpr (!std::is_trivially_destructible_v<std::decay_t<decltype(key)>>) {
                std::destroy_at(&key);
            }
        };

        (destroy_key_f(std::get<I>(tuple)), ...);
    };

    if (auto entry = originalHashEntry_[index]; entry != invalid_hash_entry_v) {
        auto& [keys, value] = data_[index];

        destroy_keys_f(std::make_index_sequence<sizeof...(Key)>{}, keys);

        if constexpr (!std::is_trivially_destructible_v<DataType>) {
            std::destroy_at(&value);
        }

        originalHashEntry_[index] = invalid_hash_entry_v;
    }
}
} // internal

template<typename DataType, size_t TableSize, typename... Key>
class StaticHashTable : protected internal::HashTableData<DataType, TableSize, Key...>
{
public:
    using table_data_t = internal::HashTableData<DataType, TableSize, Key...>;
    using key_type = typename table_data_t::key_type;
    using value_type = typename table_data_t::value_type;

    template<bool IsConstant>
    class Iterator
    {
        friend class StaticHashTable<DataType, TableSize, Key...>;

    public:
        using data_t = std::conditional_t<IsConstant, table_data_t const*, table_data_t*>;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConstant, value_type const*, value_type*>;
        using reference = std::conditional_t<IsConstant, value_type const&, value_type&>;

        // post increment
        auto operator++(int) -> Iterator;

        // pre increment
        auto operator++() -> Iterator&;

        [[nodiscard]] auto operator*() -> reference;

        auto operator->() const -> pointer;

        friend auto operator==(Iterator const& a, Iterator const& b) -> bool
        {
            return a.hashTableData_ == b.hashTableData_ && a.index_ == b.index_;
        }

        friend auto operator!=(Iterator const& a, Iterator const& b) -> bool
        {
            return a.hashTableData_ != b.hashTableData_ || a.index_ != b.index_;
        }

    private:
        Iterator();

        Iterator(table_data_t* hashTableData, size_t pos);

        [[nodiscard]] auto index() const -> size_t { return index_; }

        [[nodiscard]] auto data() const -> table_data_t* { return hashTableData_; }

    private:
        void current();
        void next();

        table_data_t* hashTableData_;
        size_t index_;
    };

    using iterator_t = Iterator<false>;
    using const_iterator_t = Iterator<true>;

    StaticHashTable() = default;

    [[nodiscard]] auto begin() -> iterator_t { return iterator_t{ this, size_t{ 0 } }; }
    [[nodiscard]] auto begin() const -> const_iterator_t { return const_iterator_t{ this, size_t{ 0 } }; }
    [[nodiscard]] auto end() -> iterator_t { return iterator_t{ this, capacity() }; }
    [[nodiscard]] auto end() const -> const_iterator_t { return const_iterator_t{ this, capacity() }; }
    [[nodiscard]] auto cbegin() const -> const_iterator_t { return const_iterator_t{ this, size_t{ 0 } }; }
    [[nodiscard]] auto cend() const -> const_iterator_t { return const_iterator_t{ this, size_t{ 0 } }; }

    template<typename... KeyN>
    [[nodiscard]] auto find(KeyN&&... keyN) -> iterator_t
        requires(std::is_convertible_v<std::decay_t<KeyN>, Key> && ...);

    template<typename... KeyN>
    [[nodiscard]] auto find(KeyN&&... keyN) const -> const_iterator_t
        requires(std::is_convertible_v<std::decay_t<KeyN>, Key> && ...);

    template<typename... KeyN>
    [[nodiscard]] auto at(KeyN&&... keyN) const
      -> DataType const* requires(std::is_convertible_v<std::decay_t<KeyN>, Key>&&...);

    template<typename... KeyN>
    [[nodiscard]] auto at(KeyN&&... keyN) -> DataType* requires(std::is_convertible_v<std::decay_t<KeyN>, Key>&&...);

    template<typename... KeyN>
    auto remove(KeyN&&... keyN) -> iterator_t
        requires(std::is_convertible_v<std::decay_t<KeyN>, Key> && ...);

    auto remove(const_iterator_t it) -> const_iterator_t;

    auto remove(iterator_t it) -> iterator_t;

    template<typename ValueType, typename... KeyN>
    auto add(ValueType&& value, KeyN&&... keyN) -> std::pair<iterator_t, bool>
        requires(std::is_convertible_v<std::decay_t<ValueType>, DataType> &&
                 (std::is_convertible_v<std::decay_t<KeyN>, Key> && ...));

    using internal::HashTableData<DataType, TableSize, Key...>::clear;

    using internal::HashTableData<DataType, TableSize, Key...>::capacity;

    using internal::HashTableData<DataType, TableSize, Key...>::size;

private:
    template<typename... KeyN>
    [[nodiscard]] auto normalizedHash(KeyN&&... keyN) const -> size_t;

    template<typename... KeyN, size_t... I>
    [[nodiscard]] auto findEntry(std::index_sequence<I...>, KeyN&&... keyN) const -> size_t;

    [[nodiscard]] auto newEntry(size_t originalEntry) const -> size_t;

    template<typename ValueType, typename... KeyN>
    [[nodiscard]] auto addEntry(ValueType&& value, KeyN&&... keyN) -> size_t;

    template<typename... KeyN>
    [[nodiscard]] auto removeEntry(KeyN&&... keyN) -> size_t;

    template<size_t... I>
    [[nodiscard]] auto getOriginalEntry(key_type const& k, std::index_sequence<I...>) const -> size_t;

    void removeEntryInternal(size_t originalEntry, size_t entry);
};

template<typename DataType, size_t TableSize, typename... Key>
template<bool IsConstant>
StaticHashTable<DataType, TableSize, Key...>::Iterator<IsConstant>::Iterator()
  : hashTableData_{ nullptr }
  , index_{ std::numeric_limits<size_t>::max() }
{
}

template<typename DataType, size_t TableSize, typename... Key>
template<bool IsConstant>
StaticHashTable<DataType, TableSize, Key...>::Iterator<IsConstant>::Iterator(table_data_t* hashTableData, size_t pos)
  : hashTableData_{ hashTableData }
  , index_{ pos }
{
    current();
}

template<typename DataType, size_t TableSize, typename... Key>
template<bool IsConstant>
void StaticHashTable<DataType, TableSize, Key...>::Iterator<IsConstant>::current()
{
    assert(hashTableData_);
    while (index_ < hashTableData_->capacity() &&
           hashTableData_->originalHashEntries()[index_] ==
             internal::HashTableData<DataType, TableSize, Key...>::invalid_hash_entry_v) {
        index_++;
    }
}

template<typename DataType, size_t TableSize, typename... Key>
template<bool IsConstant>
void StaticHashTable<DataType, TableSize, Key...>::Iterator<IsConstant>::next()
{
    index_++;
    current();
}

template<typename DataType, size_t TableSize, typename... Key>
template<bool IsConstant>
auto StaticHashTable<DataType, TableSize, Key...>::Iterator<IsConstant>::operator*() -> reference
{
    assert(hashTableData_);
    return hashTableData_->data()[index_];
}

// post increment
template<typename DataType, size_t TableSize, typename... Key>
template<bool IsConstant>
auto StaticHashTable<DataType, TableSize, Key...>::Iterator<IsConstant>::operator++(int) -> Iterator
{
    auto tmp = *this;
    next();
    return tmp;
}

// pre increment
template<typename DataType, size_t TableSize, typename... Key>
template<bool IsConstant>
auto StaticHashTable<DataType, TableSize, Key...>::Iterator<IsConstant>::operator++() -> Iterator&
{
    next();
    return *this;
}

template<typename DataType, size_t TableSize, typename... Key>
template<bool IsConstant>
auto StaticHashTable<DataType, TableSize, Key...>::Iterator<IsConstant>::operator->() const -> pointer
{
    assert(hashTableData_);
    return &hashTableData_->data()[index_];
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename... KeyN>
auto StaticHashTable<DataType, TableSize, Key...>::normalizedHash(KeyN&&... keyN) const -> size_t
{
    auto goldendRatioCapacity = std::max(size_t{ 1 }, static_cast<size_t>(capacity() * 0.618f));
    return core::hash(Key(std::forward<KeyN>(keyN))...) % goldendRatioCapacity;
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename... KeyN, size_t... I>
auto StaticHashTable<DataType, TableSize, Key...>::findEntry(std::index_sequence<I...>, KeyN&&... keyN) const -> size_t
{
    auto foundEntry = table_data_t::invalid_hash_entry_v;

    const auto originalEntry = normalizedHash(std::forward<KeyN>(keyN)...);
    auto entryCount = table_data_t::hashEntryCounts()[originalEntry];
    assert(entryCount <= capacity());

    if (entryCount > 0) {
        auto const& hashEntries = table_data_t::originalHashEntries();
        for (auto i = originalEntry, count = capacity(); i < count; i++) {
            if (hashEntries[i] == originalEntry) {
                auto&& [key, _] = table_data_t::data()[i];

                if (((std::get<I>(key) == static_cast<Key>(keyN)) && ...)) {
                    foundEntry = i;
                    break;
                }

                if (--entryCount == 0) {
                    break;
                }
            }
        }
    }

    return foundEntry;
}

template<typename DataType, size_t TableSize, typename... Key>
auto StaticHashTable<DataType, TableSize, Key...>::newEntry(size_t originalEntry) const -> size_t
{
    auto entryCount = table_data_t::hashEntryCounts()[originalEntry];
    auto currentEntry = originalEntry;

    bool found = false;

    // find the last one element hashed to originalEntry
    // or free element after original
    while (entryCount > 0) {
        auto hashEntry = table_data_t::originalHashEntries()[currentEntry++ % capacity()];
        if (hashEntry == originalEntry) {
            entryCount--;
        } else if (hashEntry == table_data_t::invalid_hash_entry_v) {
            found = true;
            break;
        }

        assert(currentEntry < capacity());
    }

    // and the first free after
    while (!found && currentEntry < capacity()) {
        found = (table_data_t::originalHashEntries()[currentEntry % capacity()] == table_data_t::invalid_hash_entry_v);
        if (!found)
            currentEntry++;
    }

    return found ? currentEntry % capacity() : table_data_t::invalid_hash_entry_v;
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename ValueType, typename... KeyN>
auto StaticHashTable<DataType, TableSize, Key...>::addEntry(ValueType&& value, KeyN&&... keyN) -> size_t
{
    auto entry = table_data_t::invalid_hash_entry_v;

    if (size() < capacity()) {
        const auto originalEntry = normalizedHash(std::forward<KeyN>(keyN)...);
        entry = newEntry(originalEntry);

        if (entry != table_data_t::invalid_hash_entry_v) {
            new (std::launder(&table_data_t::data()[entry]))
              value_type{ std::add_const_t<key_type>{ std::forward<KeyN>(keyN)... }, std::forward<ValueType>(value) };
            table_data_t::originalHashEntries()[entry] = originalEntry;
            table_data_t::hashEntryCounts()[originalEntry]++;
            table_data_t::increaseEntryCount();
        }
    }

    return entry;
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename... KeyN>
auto StaticHashTable<DataType, TableSize, Key...>::removeEntry(KeyN&&... keyN) -> size_t
{
    const auto originalEntry = normalizedHash(std::forward<KeyN>(keyN)...);
    auto entry = findEntry(std::make_index_sequence<sizeof...(KeyN)>{}, std::forward<KeyN>(keyN)...);

    removeEntryInternal(originalEntry, entry);

    return entry;
}

template<typename DataType, size_t TableSize, typename... Key>
template<size_t... I>
auto StaticHashTable<DataType, TableSize, Key...>::getOriginalEntry(key_type const& k,
                                                                    std::index_sequence<I...>) const -> size_t
{
    return normalizedHash(std::get<I>(k)...);
}

template<typename DataType, size_t TableSize, typename... Key>
void StaticHashTable<DataType, TableSize, Key...>::removeEntryInternal(size_t originalEntry, size_t entry)
{
    if (entry != table_data_t::invalid_hash_entry_v) {
        assert(table_data_t::hashEntryCounts()[originalEntry] > 0);
        table_data_t::destroyEntryData(entry);
        table_data_t::hashEntryCounts()[originalEntry]--;
        table_data_t::decreaseEntryCount();
    }
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename... KeyN>
auto StaticHashTable<DataType, TableSize, Key...>::find(KeyN&&... keyN) -> iterator_t
    requires(std::is_convertible_v<std::decay_t<KeyN>, Key> && ...)
{
    auto entry = findEntry(std::make_index_sequence<sizeof...(KeyN)>{}, std::forward<KeyN>(keyN)...);
    return (entry == table_data_t::invalid_hash_entry_v) ? end() : iterator_t{ this, entry };
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename... KeyN>
auto StaticHashTable<DataType, TableSize, Key...>::find(KeyN&&... keyN) const -> const_iterator_t
    requires(std::is_convertible_v<std::decay_t<KeyN>, Key> && ...)
{
    auto entry = findEntry(std::make_index_sequence<sizeof...(KeyN)>{}, std::forward<KeyN>(keyN)...);
    return (entry == table_data_t::invalid_hash_entry_v) ? cend() : const_iterator_t{ this, entry };
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename... KeyN>
auto StaticHashTable<DataType, TableSize, Key...>::at(KeyN&&... keyN) const
  -> DataType const* requires(std::is_convertible_v<std::decay_t<KeyN>, Key>&&...) {
    auto entry = findEntry(std::make_index_sequence<sizeof...(KeyN)>{}, std::forward<KeyN>(keyN)...);
    return (entry == table_data_t::invalid_hash_entry_v) ? nullptr : &table_data_t::data()[entry].second;
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename... KeyN>
auto StaticHashTable<DataType, TableSize, Key...>::at(KeyN&&... keyN)
  -> DataType* requires(std::is_convertible_v<std::decay_t<KeyN>, Key>&&...) {
    return const_cast<DataType*>(std::as_const(*this).at(std::forward<KeyN>(keyN)...));
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename... KeyN>
auto StaticHashTable<DataType, TableSize, Key...>::remove(KeyN&&... keyN) -> iterator_t
    requires(std::is_convertible_v<std::decay_t<KeyN>, Key> && ...)
{
    auto entry = removeEntry(std::forward<KeyN>(keyN)...);
    return (entry != table_data_t::invalid_hash_entry_v) ? iterator_t{ this, entry++ } : end();
}

template<typename DataType, size_t TableSize, typename... Key>
auto StaticHashTable<DataType, TableSize, Key...>::remove(const_iterator_t it) -> const_iterator_t
{
    assert(it != cend());

    const auto originalEntry = getOriginalEntry((*it).first, std::make_index_sequence<sizeof...(Key)>{});
    auto entry = it.index();

    removeEntryInternal(originalEntry, entry);

    return (entry != table_data_t::invalid_hash_entry_v) ? const_iterator_t{ this, entry++ } : cend();
}

template<typename DataType, size_t TableSize, typename... Key>
auto StaticHashTable<DataType, TableSize, Key...>::remove(iterator_t it) -> iterator_t
{
    assert(it != end());

    const auto originalEntry = getOriginalEntry((*it).first, std::make_index_sequence<sizeof...(Key)>{});
    auto entry = it.index();

    removeEntryInternal(originalEntry, entry);

    return (entry != table_data_t::invalid_hash_entry_v) ? iterator_t{ this, entry++ } : end();
}

template<typename DataType, size_t TableSize, typename... Key>
template<typename ValueType, typename... KeyN>
auto StaticHashTable<DataType, TableSize, Key...>::add(ValueType&& value, KeyN&&... keyN) -> std::pair<iterator_t, bool>
    requires(std::is_convertible_v<std::decay_t<ValueType>, DataType> &&
             (std::is_convertible_v<std::decay_t<KeyN>, Key> && ...))
{
    auto it = end();
    auto added = false;

    if (auto entry = findEntry(std::make_index_sequence<sizeof...(KeyN)>{}, std::forward<KeyN>(keyN)...);
        entry != table_data_t::invalid_hash_entry_v) {
        it = iterator_t{ this, entry };
    } else {
        auto entryNew = addEntry(std::forward<ValueType>(value), std::forward<KeyN>(keyN)...);
        it = iterator_t{ this, entryNew };
        added = (entryNew != table_data_t::invalid_hash_entry_v);
    }

    return std::pair{ it, added };
}
}

#endif // HASHTABLE_H
