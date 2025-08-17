#include "../../lib/service.h"
#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <tlhelp32.h>
#include <sstream>

#pragma comment(lib, "Shlwapi.lib")

namespace fs = std::filesystem;

// Convert string to lowercase
std::string to_lower(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

// Get common directories where applications are typically installed
std::vector<fs::path> get_common_dirs() {
    std::vector<fs::path> dirs;
    char* programFiles = getenv("ProgramFiles");
    char* programFilesX86 = getenv("ProgramFiles(x86)");
    char* localAppData = getenv("LOCALAPPDATA");
    char* userProfile = getenv("USERPROFILE");

    if (programFiles) dirs.push_back(programFiles);
    if (programFilesX86) dirs.push_back(programFilesX86);
    if (localAppData) {
        dirs.push_back(std::string(localAppData) + "\\Programs");
        dirs.push_back(std::string(localAppData) + "\\Microsoft\\WindowsApps");
    }
    if (userProfile) {
        dirs.push_back(std::string(userProfile) + "\\AppData\\Local\\Programs");
        dirs.push_back(std::string(userProfile) + "\\Desktop");
    }

    return dirs;
}

// Find process ID by name
std::vector<DWORD> find_process_ids(const std::string& input_name) {
    std::string target = to_lower(input_name);
    if (target.size() >= 4) {
        std::string tail = target.substr(target.size() - 4);
        if (tail == ".exe" || tail == ".lnk") {
            target = target.substr(0, target.size() - 4);
        }
    }

    std::vector<DWORD> pids;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return pids;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snapshot, &pe32)) {
        do {
            std::string proc_name = to_lower(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(pe32.szExeFile));
            if (proc_name.find(target) != std::string::npos) {
                pids.push_back(pe32.th32ProcessID);
            }
        } while (Process32NextW(snapshot, &pe32));
    }

    CloseHandle(snapshot);
    return pids;
}

// Main function to stop a process
std::string stop_process(const std::string& raw_input) {
    std::string input = raw_input;
    input.erase(0, input.find_first_not_of(" \t\r\n"));
    input.erase(input.find_last_not_of(" \t\r\n") + 1);

    if (input.empty()) {
        return "❌ No process name or PID provided.";
    }

    // Check if input is a PID
    if (std::all_of(input.begin(), input.end(), ::isdigit)) {
        int pid = std::stoi(input);
        std::ostringstream command;
        command << "taskkill /PID " << pid << " /F";
        FILE* pipe = _popen(command.str().c_str(), "r");
        if (!pipe) {
            return "❌ Failed to run taskkill for PID: " + input;
        }

        char buffer[128];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        _pclose(pipe);

        if (result.find("SUCCESS") != std::string::npos || result.find("terminated") != std::string::npos) {
            return "✅ Stopped process with PID: " + input;
        } else {
            return "❌ Failed to stop process with PID: " + input + ": " + result;
        }
    }

    // If not a PID, assume it's a process name
    std::vector<DWORD> pids = find_process_ids(input);
    if (pids.empty()) {
        return "❌ Process not found: " + input;
    }

    std::string result;
    for (DWORD pid : pids) {
        std::ostringstream command;
        command << "taskkill /PID " << pid << " /F";
        FILE* pipe = _popen(command.str().c_str(), "r");
        if (!pipe) {
            result += "❌ Failed to run taskkill for PID: " + std::to_string(pid) + "\n";
            continue;
        }

        char buffer[128];
        std::string cmd_output;
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            cmd_output += buffer;
        }
        _pclose(pipe);

        if (cmd_output.find("SUCCESS") != std::string::npos || cmd_output.find("terminated") != std::string::npos) {
            result += "✅ Stopped process with PID: " + std::to_string(pid) + "\n";
        } else {
            result += "❌ Failed to stop process with PID: " + std::to_string(pid) + ": " + cmd_output + "\n";
        }
    }

    return result.empty() ? "❌ No processes stopped." : result;
}