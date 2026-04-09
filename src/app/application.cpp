#include "app/application.hpp"

#include <cstdlib>
#include <utility>

#include "config/app_config.hpp"
#include "http/http_server.hpp"
#include "logging/logger.hpp"

namespace app {

Application::Application(std::filesystem::path config_path)
    : config_path_(std::move(config_path)) {}

int Application::run() {
    auto config = config::AppConfig::load(config_path_);
    auto logger = logging::Logger::build(config.service_name, config.logging.level);
    http::HttpServer server(std::move(config), std::move(logger));
    server.start();
    return EXIT_SUCCESS;
}

}  // namespace app
