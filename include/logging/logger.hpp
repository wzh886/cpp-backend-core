#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace logging {

class Logger {
  public:
    Logger(std::string service_name, std::string level);

    static Logger build(std::string service_name, std::string level);

    void info(std::string event, std::map<std::string, std::string> fields = {}) const;
    void warn(std::string event, std::map<std::string, std::string> fields = {}) const;
    void error(std::string event, std::map<std::string, std::string> fields = {}) const;
    void debug(std::string event, std::map<std::string, std::string> fields = {}) const;

  private:
    void log(std::string level, std::string event, std::map<std::string, std::string> fields) const;
    [[nodiscard]] bool should_log(const std::string& level) const;

    std::string service_name_;
    std::string level_;
    std::shared_ptr<std::mutex> sink_mutex_;
};

std::string json_escape(const std::string& input);
std::string current_timestamp_epoch_ms();

}  // namespace logging
