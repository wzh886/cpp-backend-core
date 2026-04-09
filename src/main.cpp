#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "app/application.hpp"
#include "core/app_error.hpp"

int main(int argc, char** argv) {
    try {
        const auto config_path = argc > 1 ? std::filesystem::path(argv[1])
                                          : std::filesystem::path("config/app.json");
        app::Application application(config_path);
        return application.run();
    } catch (const core::AppError& error) {
        std::cerr << "Application error [" << error.code() << "]: " << error.what() << '\n';
        return EXIT_FAILURE;
    } catch (const std::exception& error) {
        std::cerr << "Unhandled error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
