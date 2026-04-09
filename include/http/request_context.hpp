#pragma once

#include <map>
#include <string>

namespace http {

class RequestContext {
  public:
    RequestContext(std::string method, std::string path, std::string remote_addr);

    [[nodiscard]] const std::string& request_id() const noexcept;
    [[nodiscard]] const std::string& method() const noexcept;
    [[nodiscard]] const std::string& path() const noexcept;
    [[nodiscard]] std::string client_ip() const;
    [[nodiscard]] std::map<std::string, std::string> base_log_fields() const;

  private:
    std::string request_id_;
    std::string method_;
    std::string path_;
    std::string remote_addr_;
};

std::string generate_request_id();

}  // namespace http
