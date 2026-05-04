#pragma once

#include <string>
#include <Windows.h>
#include <optional>
#include <chrono>
#include <iomanip>
#include <sstream>


namespace utils 
{
    inline std::string utf16_to_utf8(const std::wstring& wstr) {
        if (wstr.empty()) return {};
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), strTo.data(), size_needed, nullptr, nullptr);
        return strTo;
    }

    inline std::wstring utf8_to_utf16(const std::string& str) {
        if (str.empty()) return {};
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), wstrTo.data(), size_needed);
        return wstrTo;
    }

    template<typename T>
    inline std::optional<std::string> utf16_to_utf8_opt(const std::optional<T>& val) {
        if (!val.has_value()) return std::nullopt;
        return utf16_to_utf8(*val);
    }

    template<typename T>
    inline std::optional<std::wstring> utf8_to_utf16_opt(const std::optional<T>& val) {
        if (!val.has_value()) return std::nullopt;
        return utf8_to_utf16(*val);
    }

    inline std::string toIsoString(
        const std::chrono::system_clock::time_point& tp
    ) {
        std::time_t tt = std::chrono::system_clock::to_time_t(tp);
        std::tm tm{};

#ifdef _WIN32
        gmtime_s(&tm, &tt);
#else
        gmtime_r(&tt, &tm);
#endif

        std::ostringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
}
