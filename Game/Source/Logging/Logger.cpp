#include "Logger.h"
#include <memory>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace engine {

std::shared_ptr<spdlog::logger> Logger::m_core_logger_{nullptr};
std::shared_ptr<spdlog::logger> Logger::m_client_logger_{nullptr};

void Logger::Init() {
    spdlog::set_pattern("%^[%T] %n: %v%$");
    m_core_logger_ = spdlog::stdout_color_mt("Core");
    m_client_logger_ = spdlog::stdout_color_mt("Client");

#ifdef DEBUG
    m_core_logger_->set_level(spdlog::level::trace);
    m_client_logger_->set_level(spdlog::level::trace);
#else
    m_core_logger_->set_level(spdlog::level::err);
    m_client_logger_->set_level(spdlog::level::err);
#endif
}

std::shared_ptr<spdlog::logger> Logger::core_logger() {
    return m_core_logger_;
}

std::shared_ptr<spdlog::logger> Logger::client_logger() {
    return m_client_logger_;
}

}