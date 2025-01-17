#pragma once

#include <memory>
#include "spdlog/spdlog.h"

namespace engine {

class Logger {
    static std::shared_ptr<spdlog::logger> m_core_logger_;
    static std::shared_ptr<spdlog::logger> m_client_logger_;
public:
    static void Init();
    
    static std::shared_ptr<spdlog::logger> core_logger();
    static std::shared_ptr<spdlog::logger> client_logger();
};

#define ENGINE_LOG_INFO(...) engine::Logger::core_logger()->info(__VA_ARGS__);
#define ENGINE_LOG_WARN(...) engine::Logger::core_logger()->warn(__VA_ARGS__);
#define ENGINE_LOG_ERROR(...) engine::Logger::core_logger()->error(__VA_ARGS__);
#define ENGINE_LOG_DEBUG(...) engine::Logger::core_logger()->debug(__VA_ARGS__);
#define ENGINE_LOG_TRACE(...) engine::Logger::client_logger()->trace(__VA_ARGS__);

#define APP_LOG_INFO(...) engine::Logger::client_logger()->info(__VA_ARGS__);
#define APP_LOG_WARN(...) engine::Logger::client_logger()->warn(__VA_ARGS__);
#define APP_LOG_ERROR(...) engine::Logger::client_logger()->error(__VA_ARGS__);
#define APP_LOG_DEBUG(...) engine::Logger::client_logger()->debug(__VA_ARGS__);
#define APP_LOG_TRACE(...) engine::Logger::client_logger()->trace(__VA_ARGS__);

}