#pragma once

#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace File {
using Path = std::filesystem::path;

static inline bool readContent(Path file_path, std::string &source) {
  std::ifstream f(file_path);
  if (!f)
    return false;
  source.assign(std::istreambuf_iterator<char>(f), {});
  return true;
}

static inline bool readContentBytes(Path file_path, std::vector<char> &v) {
  std::ifstream file(file_path, std::ios::binary |
                                    std::ios::ate); // open at end to get size
  if (!file)
    return false;

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  v.reserve(size);
  v.assign(std::istreambuf_iterator<char>(file), {});
  return true;
}

}; /* namespace File */