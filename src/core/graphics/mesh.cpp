#include <core/graphics/mesh.hpp>
#include <core/logging.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <glm/glm.hpp>
#include <algorithm>

#include <tiny_gltf.h>

namespace Engine {

// small key for vertex deduplication
struct Key {
  int vi; // position index (zero-based) or -1
  int ti; // texcoord index or -1
  int ni; // normal index or -1

  bool operator==(Key const& o) const noexcept {
    return vi == o.vi && ti == o.ti && ni == o.ni;
  }
};

struct KeyHash {
  std::size_t operator()(const Key &k) const noexcept {
    uint64_t a = static_cast<uint64_t>(static_cast<int64_t>(k.vi) + 0x9e3779b97f4a7c15LL);
    uint64_t b = static_cast<uint64_t>(static_cast<int64_t>(k.ti) + 0x9e3779b97f4a7c15LL);
    uint64_t c = static_cast<uint64_t>(static_cast<int64_t>(k.ni) + 0x9e3779b97f4a7c15LL);
    a ^= b + 0x9e3779b97f4a7c15ULL + (a << 6) + (a >> 2);
    a ^= c + 0x9e3779b97f4a7c15ULL + (a << 6) + (a >> 2);
    return static_cast<std::size_t>(a);
  }
};

// convert OBJ index (1-based, can be negative) -> zero-based, or -1 if absent
static int objIndexToZeroBased(int idx, std::size_t vec_size) noexcept {
  if (idx > 0) return idx - 1;
  if (idx < 0) return static_cast<int>(static_cast<long long>(vec_size) + idx);
  return -1;
}

// parse face token like "v", "v/t", "v//n", "v/t/n"
static void parseFaceToken(std::string_view token, int& vi_out, int& ti_out, int& ni_out) {
  vi_out = ti_out = ni_out = -1;
  size_t p1 = token.find('/');
  if (p1 == std::string_view::npos) {
    vi_out = std::stoi(std::string(token));
    return;
  }

  if (p1 > 0) vi_out = std::stoi(std::string(token.substr(0, p1)));

  size_t p2 = token.find('/', p1 + 1);
  if (p2 == std::string_view::npos) {
    std::string_view t = token.substr(p1 + 1);
    if (!t.empty()) ti_out = std::stoi(std::string(t));
    return;
  }

  if (p2 == p1 + 1) {
    std::string_view n = token.substr(p2 + 1);
    if (!n.empty()) ni_out = std::stoi(std::string(n));
  }
  else {
    std::string_view t = token.substr(p1 + 1, p2 - (p1 + 1));
    std::string_view n = token.substr(p2 + 1);
    if (!t.empty()) ti_out = std::stoi(std::string(t));
    if (!n.empty()) ni_out = std::stoi(std::string(n));
  }
}

std::optional<Mesh> Mesh::fromOBJ(File::Path path) {
  std::ifstream ifs(path);
  if (!ifs) return std::nullopt;

  struct Position {
    glm::vec3 pos;
    glm::vec3 color;
  };

  std::vector<Position> positions;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;

  Mesh mesh;
  mesh.vertices.clear();
  mesh.indices.clear();

  std::unordered_map<Key, Index, KeyHash> unique;
  unique.reserve(1024);

  std::string line;
  while (std::getline(ifs, line)) {
    if (line.empty()) continue;

    size_t pos = 0;
    while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) ++pos;
    if (pos >= line.size()) continue;

    if (line.compare(pos, 2, "v ") == 0) {
      std::istringstream ss(line.substr(pos + 2));
      glm::vec3 p, c(1.0f);
      ss >> p.x >> p.y >> p.z;
      if (!(ss >> c.r >> c.g >> c.b)) {
        c = glm::vec3(1.0f); // default to white if no color
      }
      positions.push_back({p, c});
    }
    else if (line.compare(pos, 3, "vt ") == 0) {
      std::istringstream ss(line.substr(pos + 3));
      glm::vec2 t; ss >> t.x >> t.y;
      uvs.push_back(t);
    }
    else if (line.compare(pos, 3, "vn ") == 0) {
      std::istringstream ss(line.substr(pos + 3));
      glm::vec3 n; ss >> n.x >> n.y >> n.z;
      normals.push_back(n);
    }
    else if (line.compare(pos, 2, "f ") == 0) {
      std::istringstream ss(line.substr(pos + 2));
      std::vector<Key> face;
      std::string token;
      while (ss >> token) {
        int vi=0, ti=0, ni=0;
        parseFaceToken(token, vi, ti, ni);
        int pvi = (vi != 0) ? objIndexToZeroBased(vi, positions.size()) : -1;
        int pti = (ti != 0) ? objIndexToZeroBased(ti, uvs.size()) : -1;
        int pni = (ni != 0) ? objIndexToZeroBased(ni, normals.size()) : -1;
        face.push_back(Key{pvi, pti, pni});
      }

      if (face.size() < 3) continue;

      glm::vec3 face_normal(0.0f);
      bool need_face_normal = std::ranges::any_of(face, [](Key const& k){ return k.ni < 0; });
      if (need_face_normal && face[0].vi >= 0 && face[1].vi >= 0 && face[2].vi >= 0) {
        const glm::vec3& p0 = positions[face[0].vi].pos;
        const glm::vec3& p1 = positions[face[1].vi].pos;
        const glm::vec3& p2 = positions[face[2].vi].pos;
        face_normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
      }

      for (size_t i = 1; i + 1 < face.size(); ++i) {
        Key tri[3] = { face[0], face[i], face[i+1] };
        for (auto& key : tri) {
          auto it = unique.find(key);
          if (it == unique.end()) {
            Mesh::Vertex v{};
            if (key.vi >= 0 && static_cast<size_t>(key.vi) < positions.size()) {
              v.position = positions[key.vi].pos;
              v.color    = positions[key.vi].color;
            }
            v.uv = (key.ti >= 0 && static_cast<size_t>(key.ti) < uvs.size()) ? uvs[key.ti] : glm::vec2(0.0f);
            if (key.ni >= 0 && static_cast<size_t>(key.ni) < normals.size())
              v.normal = normals[key.ni];
            else
              v.normal = (face_normal != glm::vec3(0.0f)) ? face_normal : glm::vec3(0,0,1);

            Index newIndex = static_cast<Index>(mesh.vertices.size());
            mesh.vertices.push_back(v);
            unique.emplace(key, newIndex);
            mesh.indices.push_back(newIndex);
          } else
            mesh.indices.push_back(it->second);
        }
      }
    }
  }

  return mesh;
}

} // namespace Engine
