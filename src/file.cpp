#include "lavi/lang/file.hpp"

namespace lavi {
  namespace lang {
    file::file(std::filesystem::path path)
    {
        this->path = path;
        stream = std::fstream(path);
    }

    file::file(const file& other)
    {
        stream = std::fstream(other.path);
        path = other.path;
    }

    void file::write(const std::string& content) {
      if (!stream.is_open()) {
        throw std::runtime_error("Failed to open file: " + path.string());
      }

      stream << content;
      stream.flush();
    }

    bool file::exists(const std::filesystem::path& path) {
      std::ifstream f(path.c_str());
      return f.good();
    }

    file file::create(const std::filesystem::path& path) {
      std::ofstream f(path.c_str());
      f.close();
      return file(path);
    }

    std::string file::read(const std::filesystem::path& path) {
      std::ifstream f(path);

      if (!f.is_open()) {
        throw std::runtime_error("Failed to open file: " + path.string());
      }

      f.seekg(0, std::ios::end);

      size_t len = f.tellg();
      f.seekg(0, std::ios::beg);

      std::string content;
      content.resize(len);

      f.read(content.data(), len);

      return content;
    }

    std::vector<std::string> file::read_lines(const std::filesystem::path& path) {
      std::vector<std::string> lines;
      std::ifstream f(path);
      std::string line;
      while (std::getline(f, line)) {
        lines.push_back(line);
      }
      return lines;
    }
  }
}