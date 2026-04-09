#include <cstdlib>
#include <fstream>
#include <iostream>

#include "config/app_config.hpp"

int main() {
    const auto path = std::filesystem::temp_directory_path() / "cpp_backend_core_config_test.json";
    std::ofstream output(path);
    output << R"({
        "service_name": "test-service",
        "environment": "test",
        "version": "9.9.9",
        "server": {"host": "127.0.0.1", "port": 9090},
        "logging": {"level": "debug"}
    })";
    output.close();

    const auto config = config::AppConfig::load(path);
    std::filesystem::remove(path);

    if (config.service_name != "test-service" ||
        config.environment != "test" ||
        config.version != "9.9.9" ||
        config.server.host != "127.0.0.1" ||
        config.server.port != 9090 ||
        config.logging.level != "debug") {
        std::cerr << "config_test failed\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
