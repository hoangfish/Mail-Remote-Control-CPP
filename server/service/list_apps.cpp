#include "../../lib/service.h"
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <set>
#include <sstream>

// Hàm chuyển từ wstring (WCHAR*) sang string (UTF-8)
std::string wstring_to_string(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(
        CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr
    );
    if (size_needed <= 0) return "";

    std::string result(size_needed - 1, 0); // -1 vì không cần null terminator
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size_needed, nullptr, nullptr);
    return result;
}

std::string list_apps() {
    std::set<std::string> apps;
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return "Error creating process snapshot.";
    }

    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return "Error reading process list.";
    }

    do {
        std::wstring wideName(pe32.szExeFile);
        std::string name = wstring_to_string(wideName);
        if (!name.empty()) {
            apps.insert(name);
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    if (apps.empty()) {
        return "No running apps found.";
    }

    std::ostringstream oss;
    for (const auto& app : apps) {
        oss << app << "\n";
    }

    return oss.str();
}
