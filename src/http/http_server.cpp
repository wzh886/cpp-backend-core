#include "http/http_server.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "http/request_context.hpp"
#include "logging/logger.hpp"

namespace http {

namespace {

struct ParsedRequest {
    std::string method;
    std::string path;
    std::string http_version;
    std::map<std::string, std::string> headers;
    std::string body;
};

std::string trim(std::string value) {
    auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::string json_string(const std::string& value) {
    return '"' + logging::json_escape(value) + '"';
}

bool looks_like_json_value(const std::string& body) {
    const auto first = body.find_first_not_of(" \t\r\n");
    const auto last = body.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || last == std::string::npos) {
        return false;
    }
    const char start = body[first];
    const char end = body[last];
    return (start == '{' && end == '}') ||
           (start == '[' && end == ']') ||
           (start == '"' && end == '"') ||
           std::isdigit(static_cast<unsigned char>(start)) || start == '-' ||
           body.substr(first, 4) == "true" || body.substr(first, 5) == "false" || body.substr(first, 4) == "null";
}

std::string response_json(int status, const std::string& body) {
    std::ostringstream out;
    out << "HTTP/1.1 " << status << (status >= 400 ? " Error" : " OK") << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return out.str();
}

ParsedRequest parse_request(int client_fd) {
    std::string raw;
    char buffer[4096];

    while (raw.find("\r\n\r\n") == std::string::npos) {
        const auto received = ::recv(client_fd, buffer, sizeof(buffer), 0);
        if (received <= 0) {
            throw core::AppError(400, "http.invalid_request", "failed to read request head");
        }
        raw.append(buffer, static_cast<std::size_t>(received));
        if (raw.size() > 1024 * 1024) {
            throw core::AppError(413, "http.request_too_large", "request head exceeded limit");
        }
    }

    const auto split = raw.find("\r\n\r\n");
    std::string head = raw.substr(0, split);
    std::string body = raw.substr(split + 4);

    std::istringstream stream(head);
    ParsedRequest request;
    std::string start_line;
    std::getline(stream, start_line);
    if (!start_line.empty() && start_line.back() == '\r') {
        start_line.pop_back();
    }

    std::istringstream start(start_line);
    start >> request.method >> request.path >> request.http_version;
    if (request.method.empty() || request.path.empty()) {
        throw core::AppError(400, "http.invalid_request_line", "missing method or path");
    }

    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        const auto pos = line.find(':');
        if (pos == std::string::npos) {
            continue;
        }
        request.headers[trim(line.substr(0, pos))] = trim(line.substr(pos + 1));
    }

    std::size_t content_length = 0;
    if (const auto it = request.headers.find("Content-Length"); it != request.headers.end()) {
        content_length = static_cast<std::size_t>(std::stoul(it->second));
    }

    while (body.size() < content_length) {
        const auto received = ::recv(client_fd, buffer, sizeof(buffer), 0);
        if (received <= 0) {
            throw core::AppError(400, "http.truncated_body", "request body ended before content-length");
        }
        body.append(buffer, static_cast<std::size_t>(received));
    }

    request.body = body.substr(0, content_length);
    return request;
}

std::string error_body(const core::AppError& error, const RequestContext& ctx) {
    std::ostringstream out;
    out << "{\"error\":{"
        << "\"code\":" << json_string(error.code()) << ','
        << "\"message\":" << json_string(error.what()) << ','
        << "\"request_id\":" << json_string(ctx.request_id()) << ','
        << "\"details\":{";

    bool first = true;
    for (const auto& [key, value] : error.details()) {
        if (!first) {
            out << ',';
        }
        first = false;
        out << json_string(key) << ':' << json_string(value);
    }
    out << "}}}";
    return out.str();
}

std::string health_body(const config::AppConfig& config, const RequestContext& ctx) {
    std::ostringstream out;
    out << "{"
        << "\"status\":\"ok\"," 
        << "\"service\":" << json_string(config.service_name) << ','
        << "\"environment\":" << json_string(config.environment) << ','
        << "\"version\":" << json_string(config.version) << ','
        << "\"request_id\":" << json_string(ctx.request_id())
        << '}';
    return out.str();
}

std::string info_body(const config::AppConfig& config, const RequestContext& ctx) {
    std::ostringstream out;
    out << "{"
        << "\"service\":{"
        << "\"name\":" << json_string(config.service_name) << ','
        << "\"environment\":" << json_string(config.environment) << ','
        << "\"version\":" << json_string(config.version)
        << "},"
        << "\"request\":{"
        << "\"id\":" << json_string(ctx.request_id()) << ','
        << "\"method\":" << json_string(ctx.method()) << ','
        << "\"path\":" << json_string(ctx.path()) << ','
        << "\"client_ip\":" << json_string(ctx.client_ip())
        << "},"
        << "\"capabilities\":[\"health\",\"structured_logging\",\"json_api\",\"config_overrides\"]"
        << '}';
    return out.str();
}

std::string echo_body(const ParsedRequest& request, const RequestContext& ctx) {
    if (request.body.empty()) {
        throw core::AppError(400, "http.empty_body", "request body must not be empty");
    }
    if (!looks_like_json_value(request.body)) {
        throw core::AppError(400, "http.invalid_json", "request body must be valid-looking JSON");
    }

    std::ostringstream out;
    out << "{"
        << "\"data\":" << request.body << ','
        << "\"meta\":{"
        << "\"request_id\":" << json_string(ctx.request_id()) << ','
        << "\"echoed_at\":" << json_string(logging::current_timestamp_epoch_ms())
        << "}}";
    return out.str();
}

void send_all(int fd, const std::string& payload) {
    std::size_t offset = 0;
    while (offset < payload.size()) {
        const auto written = ::send(fd, payload.data() + offset, payload.size() - offset, 0);
        if (written <= 0) {
            return;
        }
        offset += static_cast<std::size_t>(written);
    }
}

}  // namespace

HttpServer::HttpServer(config::AppConfig config, logging::Logger logger)
    : config_(std::move(config)), logger_(std::move(logger)) {}

void HttpServer::start() {
    const int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw core::AppError(500, "server.socket_failed", "unable to create socket");
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config_.server.port);
    addr.sin_addr.s_addr = config_.server.host == "0.0.0.0"
        ? INADDR_ANY
        : inet_addr(config_.server.host.c_str());

    if (::bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(server_fd);
        throw core::AppError(500, "server.bind_failed", "failed to bind HTTP server",
                             {{"host", config_.server.host}, {"port", std::to_string(config_.server.port)}});
    }

    if (::listen(server_fd, SOMAXCONN) < 0) {
        ::close(server_fd);
        throw core::AppError(500, "server.listen_failed", "failed to listen for HTTP connections");
    }

    logger_.info("server.starting", {{"host", config_.server.host}, {"port", std::to_string(config_.server.port)}});

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        const int client_fd = ::accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client_fd < 0) {
            continue;
        }

        std::thread([this, client_fd, client_addr]() {
            const std::string remote_ip = inet_ntoa(client_addr.sin_addr);
            try {
                const auto request = parse_request(client_fd);
                const RequestContext ctx(request.method, request.path, remote_ip);

                std::string body;
                int status = 200;

                if (request.method == "GET" && request.path == "/healthz") {
                    body = health_body(config_, ctx);
                } else if (request.method == "GET" && request.path == "/api/v1/info") {
                    body = info_body(config_, ctx);
                } else if (request.method == "POST" && request.path == "/api/v1/echo") {
                    body = echo_body(request, ctx);
                } else {
                    throw core::AppError(404, "http.route_not_found", "route not found",
                                         {{"method", request.method}, {"path", request.path}});
                }

                auto fields = ctx.base_log_fields();
                fields["status"] = std::to_string(status);
                logger_.info("request.completed", std::move(fields));
                send_all(client_fd, response_json(status, body));
            } catch (const core::AppError& error) {
                const RequestContext ctx("UNKNOWN", "UNKNOWN", remote_ip);
                auto fields = ctx.base_log_fields();
                fields["status"] = std::to_string(error.http_status());
                fields["error_code"] = error.code();
                logger_.warn("request.failed", std::move(fields));
                send_all(client_fd, response_json(error.http_status(), error_body(error, ctx)));
            } catch (const std::exception& error) {
                const RequestContext ctx("UNKNOWN", "UNKNOWN", remote_ip);
                const core::AppError app_error(500, "http.internal_error", "unexpected server error", {{"reason", error.what()}});
                auto fields = ctx.base_log_fields();
                fields["status"] = "500";
                fields["error_code"] = app_error.code();
                logger_.error("request.failed", std::move(fields));
                send_all(client_fd, response_json(500, error_body(app_error, ctx)));
            }
            ::close(client_fd);
        }).detach();
    }
}

}  // namespace http
