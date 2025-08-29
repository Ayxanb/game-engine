#pragma once

#include <vector>

namespace Engine {

struct Material {
  uint32_t shader_handle;
};

class MaterialManager {
  std::vector<Material> materials;

public:
  MaterialManager() noexcept = default;
  ~MaterialManager() noexcept = default;

  inline uint32_t addMaterial(const Material &mat) {
    const uint32_t index = materials.size();
    materials.push_back(mat);
    return index;
  }

  inline Material &getMaterial(uint32_t index)  {
    return materials.at(index);
  }

  const Material &getMaterial(uint32_t index) const {
    return materials.at(index);
  }
};


}; /* namespace Engine */