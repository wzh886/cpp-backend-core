#include "config/app_config.hpp"

#include <cstdlib>
#include <fstream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>

#include "core/app_error.hpp"

namespace {

std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw core::AppError(500, "config.file_not_found", "unable to open config file",
                             {{"path", path.string()}});
    }
    std::ostringstream out;
    out << input.rdbuf();
    return out.str();
}

std::optional<std::string> match_string(const std::string& text, const std::string& key) {
    const std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*\\\"([^\\\"]*)\\\"");
    std::smatch match;
    if (std::regex_search(text, match, pattern)) {
        return match[1].str();
    }
    return std::nullopt;
}

std::optional<int> match_int(const std::string& text, const std::string& key) {
    const std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*([0-9]+)");
    std::smatch match;
    if (std::regex_search(text, match, pattern)) {
        return std::stoi(match[1].str());
    }
    return std::nullopt;
}

std::uint16_t checked_port(int value) {
    if (value < 1 || value > 65535) {
        throw core::AppError(500, "config.invalid_port", "server.port must be between 1 and 65535");
    }
    return static_cast<std::uint16_t>(value);
}

void apply_env_overrides(config::AppConfig& config) {
    if (const auto value = config::read_env("APP_SERVICE_NAME")) config.service_name = *value;
    if (const auto value = config::read_env("APP_ENV")) config.environment = *value;
    if (const auto value = config::read_env("APP_VERSION")) config.version = *value;
    if (const auto value = config::read_env("APP_HOST")) config.server.host = *value;
    if (const auto value = config::read_env("APP_PORT")) config.server.port = checked_port(std::stoi(*value));
    if (const auto value = config::read_env("APP_LOG_LEVEL")) config.logging.level = *value;
}

}  // namespace

namespace config {

std::optional<std::string> read_env(const char* key) {
    if (const char* raw = std::getenv(key); raw != nullptr) {
        return std::string(raw);
    }
    return std::nullopt;
}

AppConfig AppConfig::load(const std::filesystem::path& path) {
    const auto raw = read_file(path);

    AppConfig config;
    if (const auto value = match_string(raw, "service_name")) config.service_name = *value;
    if (const auto value = match_string(raw, "environment")) config.environment = *value;
    if (const auto value = match_string(raw, "version")) config.version = *value;
    if (const auto value = match_string(raw, "host")) config.server.host = *value;
    if (const auto value = match_int(raw, "port")) config.server.port = checked_port(*value);
    if (const auto value = match_int(raw, "read_timeout_seconds")) config.server.read_timeout_seconds = *value;
    if (const auto value = match_int(raw, "write_timeout_seconds")) config.server.write_timeout_seconds = *value;
    if (const auto value = match_int(raw, "idle_interval_microseconds")) config.server.idle_interval_microseconds = *value;
    if (const auto value = match_string(raw, "level")) config.logging.level = *value;

    apply_env_overrides(config);
    return config;
}

AppConfig AppConfig::load_from_environment() {
    const auto config_path = read_env("APP_CONFIG").value_or("config/app.json");
    return load(config_path);
}

}  // namespace config
