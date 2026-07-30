#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <windows.h>

enum class LogLevel {
  Info,
  Warning,
  Error,
  Critical
};

class Logger {
 public:
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  static Logger& GetInstance() {
    static Logger Instance;
    
    return Instance;
  }

  void Initialize(const std::string& logFilePath = "Engine.log");
  void Shutdown();

  void Log(LogLevel level, const std::string& msg, const char* file, int line);
 private:
  Logger() = default;
  ~Logger() {
    Shutdown();
  }

  std::ofstream m_logFile;
  bool m_isInitialized = false;

  std::string GetLevelString(LogLevel level);
};

#if defined(NDEBUG)
  #define ENGINE_LOG_INFO(msg)     ((void)0)
  #define ENGINE_LOG_WARNING(msg)  ((void)0)
  #define ENGINE_LOG_ERROR(msg)    ((void)0)
  #define ENGINE_LOG_CRITICAL(msg) ((void)0)
#else
  #define ENGINE_LOG_INFO(msg)     ::Logger::GetInstance().Log(::LogLevel::Info,     msg, __FILE__, __LINE__)
  #define ENGINE_LOG_WARNING(msg)  ::Logger::GetInstance().Log(::LogLevel::Warning,  msg, __FILE__, __LINE__)
  #define ENGINE_LOG_ERROR(msg)    ::Logger::GetInstance().Log(::LogLevel::Error,    msg, __FILE__, __LINE__)
  #define ENGINE_LOG_CRITICAL(msg) ::Logger::GetInstance().Log(::LogLevel::Critical, msg, __FILE__, __LINE__)
#endif