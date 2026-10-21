
#ifndef CYCLONITE_COMPONENTS_TRANSFORM_STORAGE
#define CYCLONITE_COMPONENTS_TRANSFORM_STORAGE

#include "enttx/componentStorageConcept.h"
#include "transform.h"
#include <vector>

namespace cyclonite::components {
class TransformStorage
{
public:
    using component_type = Transform;

    using storage_type = std::vector<component_type>;

    TransformStorage() = default;

    void reserve(size_t capacity);

    [[nodiscard]] auto capacity() const -> size_t { return storage_.capacity(); }

    [[nodiscard]] auto size() const -> size_t { return storage_.size(); }

    [[nodiscard]] auto get(uint32_t index) const -> Transform const&;

    [[nodiscard]] auto get(uint32_t index) -> Transform&;

    auto create(uint32_t index, enttx::Entity parent) -> Transform&;

    auto create(uint32_t index, enttx::Entity parent, vec3 const& position, vec3 const& scale, quat const& rotation)
      -> Transform&;

    auto create(uint32_t index, enttx::Entity parent, mat4 const& matrix) -> Transform&;

    auto create(uint32_t index) -> Transform& { return create(index, enttx::Entity{}, 0); }

    void destroy(uint32_t index);

    [[nodiscard]] auto begin() const -> std::vector<Transform>::const_iterator { return storage_.cbegin(); }

    [[nodiscard]] auto end() const -> std::vector<Transform>::const_iterator { return storage_.cend(); }

    [[nodiscard]] auto begin() -> std::vector<Transform>::iterator { return storage_.begin(); }

    [[nodiscard]] auto end() -> std::vector<Transform>::iterator { return storage_.end(); }

    [[nodiscard]] auto getFirstEntityIndex() const -> uint32_t
    {
        return componentIdxToEntityIdx_.empty() ? std::numeric_limits<uint32_t>::max()
                                                : *componentIdxToEntityIdx_.cbegin();
    }

    [[nodiscard]] auto getLastEntityIndex() const -> uint32_t
    {
        return componentIdxToEntityIdx_.empty() ? std::numeric_limits<uint32_t>::max()
                                                : *componentIdxToEntityIdx_.crbegin();
    }

    [[nodiscard]] auto getEntityIndex(uint32_t componentIdx) const -> uint32_t
    {
        return componentIdx < componentIdxToEntityIdx_.size() ? componentIdxToEntityIdx_[componentIdx]
                                                              : std::numeric_limits<uint32_t>::max();
    }

    [[nodiscard]] auto getComponentIndex(uint32_t index) const -> uint32_t {
        assert(index < entityIdxToComponent_.size());
        return entityIdxToComponent_[index];
    }

private:
    constexpr static size_t chunk_size_v = 256;

    void resizeIndicesIfNecessary(uint32_t index);

    void reserveStoreIfNecessary(uint32_t componentIdx);

    std::vector<uint32_t> entityIdxToComponent_; 
    std::vector<uint32_t> componentIdxToEntityIdx_;
    std::vector<component_type> storage_;
};
// quick test:
static_assert(enttx::ComponentStorageConcept<TransformStorage>);
}

#endif // CYCLONITE_COMPONENTS_TRANSFORM_STORAGE
