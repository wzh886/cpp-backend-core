#pragma once

#include <filesystem>

namespace app {

class Application {
  public:
    explicit Application(std::filesystem::path config_path);

    int run();

  private:
    std::filesystem::path config_path_;
};

}  // namespace app
