#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

namespace lavi {
  namespace lang {
    class file {
    private:
      std::fstream stream;
      std::filesystem::path path;
    public:
      file(std::filesystem::path path);
      file(const file& other);
      ~file() = default;
    public:
      void write(const std::string& content);
    public:
      static bool exists(const std::filesystem::path& path);
      static file create(const std::filesystem::path& path);
      static std::string read(const std::filesystem::path& path);
      static std::vector<std::string> read_lines(const std::filesystem::path& path);
    };
  }
}