#pragma once
#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include <mutex>

// Simple logger for debugging
class Logger
{
private:
    static std::ofstream& GetLogFile()
    {
        static std::ofstream logFile("graphics_engine_log.txt", std::ios::app);
        return logFile;
    }

    static std::mutex& GetMutex()
    {
        static std::mutex mtx;
        return mtx;
    }

public:
    static void Log(const std::string& message)
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        std::string fullMsg = "[LOG] " + message + "\n";
        OutputDebugStringA(fullMsg.c_str());
        GetLogFile() << fullMsg << std::flush;
    }

    static void LogError(const std::string& message, HRESULT hr = S_OK)
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        std::stringstream ss;
        ss << "[ERROR] " << message;
        if (FAILED(hr))
        {
            ss << " (HRESULT: 0x" << std::hex << hr << ")";
        }
        ss << "\n";

        OutputDebugStringA(ss.str().c_str());
        GetLogFile() << ss.str() << std::flush;

        // Also show message box for critical errors
        MessageBoxA(nullptr, ss.str().c_str(), "Error", MB_OK | MB_ICONERROR);
    }

    static void LogWarning(const std::string& message)
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        std::string fullMsg = "[WARNING] " + message + "\n";
        OutputDebugStringA(fullMsg.c_str());
        GetLogFile() << fullMsg << std::flush;
    }

    static void ClearLog()
    {
        std::ofstream logFile("graphics_engine_log.txt", std::ios::trunc);
        logFile.close();
    }
};
