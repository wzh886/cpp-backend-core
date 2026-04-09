#include "http/request_context.hpp"

#include <array>
#include <random>
#include <utility>

namespace http {

std::string generate_request_id() {
    static thread_local std::mt19937_64 rng(std::random_device{}());
    static constexpr std::array<char, 16> alphabet{
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    std::string out(16, '0');
    for (auto& ch : out) {
        ch = alphabet[rng() % alphabet.size()];
    }
    return out;
}

RequestContext::RequestContext(std::string method, std::string path, std::string remote_addr)
    : request_id_(generate_request_id()),
      method_(std::move(method)),
      path_(std::move(path)),
      remote_addr_(std::move(remote_addr)) {}

const std::string& RequestContext::request_id() const noexcept {
    return request_id_;
}

const std::string& RequestContext::method() const noexcept {
    return method_;
}

const std::string& RequestContext::path() const noexcept {
    return path_;
}

std::string RequestContext::client_ip() const {
    return remote_addr_.empty() ? "unknown" : remote_addr_;
}

std::map<std::string, std::string> RequestContext::base_log_fields() const {
    return {
        {"request_id", request_id_},
        {"method", method_},
        {"path", path_},
        {"client_ip", client_ip()}
    };
}

}  // namespace http
