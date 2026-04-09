#pragma once

#include <string>

#include "config/app_config.hpp"
#include "core/app_error.hpp"
#include "logging/logger.hpp"

namespace http {

class HttpServer {
  public:
    HttpServer(config::AppConfig config, logging::Logger logger);

    void start();

  private:
    config::AppConfig config_;
    logging::Logger logger_;
};

}  // namespace http
