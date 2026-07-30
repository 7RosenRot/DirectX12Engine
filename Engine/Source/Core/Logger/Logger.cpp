#include <Framework/Logger/Logger.hpp>

#include <chrono>
#include <iomanip>

void Logger::Initialize(const std::string& logFilePath) {
  if (m_isInitialized) {
    return;
  }

  m_logFile.open(logFilePath, std::ios::out | std::ios::trunc);
  m_isInitialized = m_logFile.is_open();
}

void Logger::Shutdown() {
  if (!m_isInitialized) {
    return;
  }

  if (m_logFile.is_open()) {
    m_logFile.close();
  }

  m_isInitialized = false;
}

void Logger::Log(LogLevel level, const std::string& msg, const char* file, int line) {
  if (!m_isInitialized) {
    return;
  }

  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::tm local{};
  if (localtime_s(&local, &time) != 0) {
    local = std::tm{};
  }

  std::string filePath(file);
  std::string fileName = filePath.substr(filePath.find_last_of("\\/") + 1);

  std::stringstream stream;
  stream << "[ " << std::put_time(&local, "%H:%M:%S") << " ] "
         << "[ " << GetLevelString(level)             << " ] "
         << "[ " << fileName << ':' << line           << " ] "
         << "- " << msg << "\n";

  std::string logString = stream.str();

  if (m_logFile.is_open()) {
    m_logFile << logString;

    m_logFile.flush();
  }

  OutputDebugStringA(logString.c_str());
}

std::string Logger::GetLevelString(LogLevel level) {
  switch (level) {
    case LogLevel::Info:
      return "INFO";
    case LogLevel::Warning:
      return "WARNING";
    case LogLevel::Error:
      return "ERROR";
    case LogLevel::Critical:
      return "CRITICAL";
  }

  return "UNKNOWN";
}