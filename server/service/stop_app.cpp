#include "../../lib/service.h"
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdio>

// Đổi tên hàm to_lower thành to_low
std::string to_low(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

// Lấy danh sách tiến trình đang chạy
std::vector<std::string> get_running_processes() {
    std::vector<std::string> processes;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return processes;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snap, &pe)) {
        do {
            processes.emplace_back(pe.szExeFile);
        } while (Process32Next(snap, &pe));
    }

    CloseHandle(snap);
    return processes;
}

// Tìm tiến trình gần đúng theo tên app, bỏ qua uninstall, plugin...
std::string find_process_match(const std::string& input) {
    std::string target = to_low(input);
    if (target.size() < 4 || target.substr(target.size() - 4) != ".exe") {
        target += ".exe";
    }

    std::vector<std::string> bad_keywords = { "uninstall", "plugin", "capture", "temp", "update" };
    std::vector<std::string> procs = get_running_processes();

    for (const auto& name : procs) {
        std::string lower = to_low(name);

        bool is_bad = false;
        for (const auto& kw : bad_keywords) {
            if (lower.find(kw) != std::string::npos) {
                is_bad = true;
                break;
            }
        }
        if (is_bad) continue;

        if (lower == target) return name; // Trùng tuyệt đối
    }

    for (const auto& name : procs) {
        std::string lower = to_low(name);

        bool is_bad = false;
        for (const auto& kw : bad_keywords) {
            if (lower.find(kw) != std::string::npos) {
                is_bad = true;
                break;
            }
        }
        if (is_bad) continue;

        if (lower.find(target) != std::string::npos) return name; // Trùng gần đúng
    }

    return "";
}

// Dừng tiến trình
std::string stop_app(const std::string& raw_input) {
    std::string app_name = raw_input;
    app_name.erase(0, app_name.find_first_not_of(" \t\r\n"));
    app_name.erase(app_name.find_last_not_of(" \t\r\n") + 1);

    if (app_name.empty()) {
        return "❌ No application name provided.";
    }

    std::string match = find_process_match(app_name);
    if (match.empty()) {
        return "❌ No matching process found for: " + app_name;
    }

    std::string command = "taskkill /f /im \"" + match + "\"";
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) return "❌ Failed to run taskkill.";

    std::ostringstream output;
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output << buffer;
    }
    int code = _pclose(pipe);
    std::string result = output.str();

    std::string upper = result;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    if (upper.find("SUCCESS") != std::string::npos) {
        return "✅ Stopped application: " + match;
    } else {
        return !result.empty() ? result : "❌ Unknown error stopping: " + match;
    }
}