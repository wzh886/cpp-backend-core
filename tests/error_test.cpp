#include <cstdlib>
#include <iostream>

#include "core/app_error.hpp"

int main() {
    const core::AppError error(422, "validation.failed", "payload rejected", {{"field", "message"}});

    if (error.http_status() != 422 ||
        error.code() != "validation.failed" ||
        std::string(error.what()) != "payload rejected" ||
        error.details().at("field") != "message") {
        std::cerr << "error_test failed\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
