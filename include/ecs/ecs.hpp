#pragma once

#include <corecrt.h>
#include <queue>
#include <span>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <cassert>
#include <tuple>
#include <algorithm>
#include <limits>

#include <core/logging.hpp>

namespace Engine::ECS {
using Entity = uint32_t;

/* Base for all component arrays */
struct IComponentArray {
  virtual ~IComponentArray() = default;
  virtual void remove(Entity) = 0;
  virtual size_t size() const = 0;
};

/* Optimized component array for cache-friendly ECS */
template <typename T>
struct ComponentArray final : IComponentArray {
  std::vector<T> components;
  std::vector<Entity> entities;
  std::unordered_map<Entity, size_t> lookup;

  ComponentArray() noexcept {
    components.reserve(64);
    entities.reserve(64);
  }

  template <typename... Args>
  void addComponent(Entity entity, Args&&... args) {
    assert(!lookup.contains(entity) && "Entity already has component!");
    LOG_DEBUG("[ECS] Adding `{}` to entity id {}", typeid(T).name(), entity);

    const size_t index = components.size();
    components.emplace_back(std::forward<Args>(args)...);
    entities.push_back(entity);
    lookup[entity] = index;
  }

  inline T *getComponent(Entity entity) {
    if (auto it = lookup.find(entity); it != lookup.end())
      return &components[it->second];
    return nullptr;
  }

  void remove(Entity entity) override {
    if (auto it = lookup.find(entity); it != lookup.end()) {
      const size_t index = it->second;
      const size_t lastIndex = components.size() - 1;

      std::swap(components[index], components[lastIndex]);
      std::swap(entities[index], entities[lastIndex]);
      lookup[entities[index]] = index;

      components.pop_back();
      entities.pop_back();
      lookup.erase(it);
      LOG_DEBUG("[ECS] Removed {} from entity {}", typeid(T).name(), entity);
    }
  }

  size_t size() const override { return components.size(); }
};

/* Unique type IDs for components */
struct ComponentTypeID {
  template <typename T>
  static constexpr uint32_t get() {
    static const uint32_t type_id = next_id++;
    return type_id;
  }
private:
  inline static uint32_t next_id = 0;
};

/* Manages all component arrays */
struct Registry {
  std::unordered_map<uint32_t, std::unique_ptr<IComponentArray>> componentArrays;

  template <typename T, typename... Args>
  void addComponent(Entity e, Args&&... args) {
    const uint32_t type_id = ComponentTypeID::get<T>();
    ensureArrayExists<T>(type_id);
    static_cast<ComponentArray<T>*>(componentArrays[type_id].get())->addComponent(e, std::forward<Args>(args)...);
  }

  template <typename T>
  T *getComponent(Entity e) {
    const uint32_t id = ComponentTypeID::get<T>();
    if (auto it = componentArrays.find(id); it != componentArrays.end())
      return static_cast<ComponentArray<T>*>(it->second.get())->getComponent(e);
    return nullptr;
  }

  template <typename T>
  void removeComponent(Entity e) {
    const uint32_t id = ComponentTypeID::get<T>();
    if (auto it = componentArrays.find(id); it != componentArrays.end())
      it->second->remove(e);
  }

  template <typename T>
  ComponentArray<T> *tryGetArray() {
    const uint32_t id = ComponentTypeID::get<T>();
    if (auto it = componentArrays.find(id); it != componentArrays.end())
      return static_cast<ComponentArray<T>*>(it->second.get());
    return nullptr;
  }

  template <typename T>
  bool hasArray() const {
    return componentArrays.contains(ComponentTypeID::get<T>());
  }

private:
  template <typename T>
  void ensureArrayExists(uint32_t type_id) {
    if (!componentArrays.contains(type_id))
      componentArrays[type_id] = std::make_unique<ComponentArray<T>>();
  }

public:
  /* View over multiple components */
  template <typename... Components>
  struct View {
    explicit View(Registry *_manager) : registry(_manager) {
      if (!(registry->hasArray<Components>() && ...)) {
        base_array = nullptr;
        return;
      }
      pickSmallestBase<Components...>();
      if (base_array)
        ((std::get<ComponentArray<Components>*>(arrays) = registry->tryGetArray<Components>()), ...);
    }

    struct Iterator {
      size_t index = 0;
      std::tuple<ComponentArray<Components>*...> arrays{};
      IComponentArray *base_array = nullptr;  // <--- change here
      Registry *registry = nullptr;

      void advance_to_valid() {
        if (!base_array) return;
        auto *concrete_base = static_cast<ComponentArray<std::tuple_element_t<0, std::tuple<Components...>>>*>(base_array);
        while (index < concrete_base->entities.size()) {
          Entity e = concrete_base->entities[index];
          if (((std::get<ComponentArray<Components>*>(arrays)->getComponent(e) != nullptr) && ...)) break;
          ++index;
        }
      }

      Iterator &operator++() { ++index; advance_to_valid(); return *this; }
      bool operator!=(const Iterator& o) const { return index != o.index || base_array != o.base_array; }

      auto operator*() const {
        auto *concrete_base = static_cast<ComponentArray<std::tuple_element_t<0, std::tuple<Components...>>>*>(base_array);
        Entity e = concrete_base->entities[index];
        return std::tuple<Entity, Components&...>(e, *std::get<ComponentArray<Components>*>(arrays)->getComponent(e)...);
      }
    };

    Iterator begin() {
      if (!base_array) return end();
      Iterator it{0, arrays, base_array, registry};
      it.advance_to_valid();
      return it;
    }

    Iterator end() {
      auto *concrete_base = base_array ? base_array : nullptr;
      return Iterator{ concrete_base ? static_cast<ComponentArray<std::tuple_element_t<0, std::tuple<Components...>>>*>(concrete_base)->entities.size() : 0,
                      arrays, base_array, registry };
    }

  private:
    Registry *registry = nullptr;
    IComponentArray *base_array = nullptr;
    std::tuple<ComponentArray<Components>*...> arrays;

    template <typename T>
    void consider_as_base(size_t &min_size) {
      if (auto *arr = registry->tryGetArray<T>()) {
        const size_t sz = arr->size();
        if (sz < min_size) {
          min_size = sz;
          base_array = arr; // now okay because it's IComponentArray*
        }
      } else {
        min_size = 0;
        base_array = nullptr;
      }
    }

    template <typename T0, typename... Ts>
    inline void pickSmallestBase() {
      size_t min_size = std::numeric_limits<size_t>::max();
      (consider_as_base<T0>(min_size), ..., consider_as_base<Ts>(min_size));
    }
  };

  template <typename... Components>
  View<Components...> view() { return View<Components...>(this); }
};

/* Entity manager */
class EntityManager {
  Entity next_id{1};
  std::vector<Entity> alive;
  std::queue<Entity> free_ids;
  std::unique_ptr<Registry> registry;

  static constexpr size_t INITIAL_ENTITY_CAPACITY = 1024;

public:
  EntityManager() noexcept : registry(std::make_unique<Registry>()) { alive.reserve(INITIAL_ENTITY_CAPACITY); }

  Entity create(auto&&... components) {
    Entity id;
    if (!free_ids.empty()) {
      id = free_ids.front();
      free_ids.pop();
    }
    else id = next_id++;

    (registry->addComponent<std::decay_t<decltype(components)>>(id, std::forward<decltype(components)>(components)), ...);
    alive.push_back(id);
    return id;
  }

  inline void destroy(Entity e) {
    free_ids.push(e);
    if (auto it = std::ranges::find(alive, e); it != alive.end()) {
      *it = alive.back();
      alive.pop_back();
    }
  }

  inline std::span<Entity> getAliveEntities() { return std::span<Entity> { alive }; }

  template <typename... Components>
  __forceinline auto view() { return registry->view<Components...>(); }
};
} // namespace Engine::ECS
