#include "logging/logger.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

namespace logging {

namespace {

int level_rank(const std::string& level) {
    if (level == "debug") return 10;
    if (level == "info") return 20;
    if (level == "warn") return 30;
    if (level == "error") return 40;
    return 20;
}

}  // namespace

Logger::Logger(std::string service_name, std::string level)
    : service_name_(std::move(service_name)),
      level_(std::move(level)),
      sink_mutex_(std::make_shared<std::mutex>()) {}

Logger Logger::build(std::string service_name, std::string level) {
    return Logger(std::move(service_name), std::move(level));
}

void Logger::info(std::string event, std::map<std::string, std::string> fields) const {
    log("info", std::move(event), std::move(fields));
}

void Logger::warn(std::string event, std::map<std::string, std::string> fields) const {
    log("warn", std::move(event), std::move(fields));
}

void Logger::error(std::string event, std::map<std::string, std::string> fields) const {
    log("error", std::move(event), std::move(fields));
}

void Logger::debug(std::string event, std::map<std::string, std::string> fields) const {
    log("debug", std::move(event), std::move(fields));
}

bool Logger::should_log(const std::string& level) const {
    return level_rank(level) >= level_rank(level_);
}

void Logger::log(std::string level, std::string event, std::map<std::string, std::string> fields) const {
    if (!should_log(level)) {
        return;
    }

    fields["event"] = std::move(event);
    fields["service"] = service_name_;
    fields["level"] = std::move(level);
    fields["timestamp_epoch_ms"] = current_timestamp_epoch_ms();

    std::ostringstream out;
    out << '{';
    bool first = true;
    for (const auto& [key, value] : fields) {
        if (!first) {
            out << ',';
        }
        first = false;
        out << '"' << json_escape(key) << '"' << ':' << '"' << json_escape(value) << '"';
    }
    out << '}';

    const std::lock_guard<std::mutex> lock(*sink_mutex_);
    std::cout << out.str() << std::endl;
}

std::string json_escape(const std::string& input) {
    std::ostringstream out;
    for (const char ch : input) {
        switch (ch) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20U) {
                    out << '?';
                } else {
                    out << ch;
                }
                break;
        }
    }
    return out.str();
}

std::string current_timestamp_epoch_ms() {
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

}  // namespace logging
