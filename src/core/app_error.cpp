#include "core/app_error.hpp"

namespace core {

AppError::AppError(int http_status, std::string code, std::string message,
                   std::map<std::string, std::string> details)
    : std::runtime_error(std::move(message)),
      http_status_(http_status),
      code_(std::move(code)),
      details_(std::move(details)) {}

int AppError::http_status() const noexcept {
    return http_status_;
}

const std::string& AppError::code() const noexcept {
    return code_;
}

const std::map<std::string, std::string>& AppError::details() const noexcept {
    return details_;
}

}  // namespace core
