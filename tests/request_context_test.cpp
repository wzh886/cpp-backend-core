#include <cstdlib>
#include <iostream>

#include "http/request_context.hpp"

int main() {
    const http::RequestContext ctx("POST", "/api/v1/echo", "127.0.0.1");
    const auto fields = ctx.base_log_fields();

    if (ctx.method() != "POST" ||
        ctx.path() != "/api/v1/echo" ||
        ctx.client_ip() != "127.0.0.1" ||
        fields.at("request_id").size() != 16) {
        std::cerr << "request_context_test failed\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
