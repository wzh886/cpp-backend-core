#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace config {

struct ServerConfig {
    std::string host{"0.0.0.0"};
    std::uint16_t port{8080};
    int read_timeout_seconds{5};
    int write_timeout_seconds{5};
    int idle_interval_microseconds{100000};
};

struct LoggingConfig {
    std::string level{"info"};
};

struct AppConfig {
    std::string service_name{"cpp-backend-core"};
    std::string environment{"development"};
    std::string version{"0.1.0"};
    ServerConfig server{};
    LoggingConfig logging{};

    static AppConfig load(const std::filesystem::path& path);
    static AppConfig load_from_environment();
};

std::optional<std::string> read_env(const char* key);

}  // namespace config
