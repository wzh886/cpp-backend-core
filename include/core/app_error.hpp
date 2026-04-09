#pragma once

#include <map>
#include <stdexcept>
#include <string>

namespace core {

class AppError : public std::runtime_error {
  public:
    AppError(int http_status, std::string code, std::string message,
             std::map<std::string, std::string> details = {});

    [[nodiscard]] int http_status() const noexcept;
    [[nodiscard]] const std::string& code() const noexcept;
    [[nodiscard]] const std::map<std::string, std::string>& details() const noexcept;

  private:
    int http_status_;
    std::string code_;
    std::map<std::string, std::string> details_;
};

}  // namespace core
