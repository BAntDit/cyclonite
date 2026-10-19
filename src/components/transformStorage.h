
#ifndef CYCLONITE_COMPONENTS_TRANSFORM_STORAGE
#define CYCLONITE_COMPONENTS_TRANSFORM_STORAGE

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

    auto create(uint32_t index, enttx::Entity parent, vec3 const& pos, vec3 const& sc, quat const& rot) -> Transform&;

private:
    void resizeIndicesIfNecessary(uint32_t index); 

    std::vector<uint32_t> entityIdxToComponent_; // maybe better use unordered map
    std::vector<uint32_t> componentIdxToEntityIdx_;
    std::vector<component_type> storage_;
};
}

#endif // CYCLONITE_COMPONENTS_TRANSFORM_STORAGE
