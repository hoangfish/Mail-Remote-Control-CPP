#include "../../lib/service.h"
#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

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

// Find application path with fuzzy matching
std::string find_app_path(const std::string& input_name) {
    std::string target = to_lower(input_name);
    if (target.size() >= 4) {
        std::string tail = target.substr(target.size() - 4);
        if (tail == ".exe" || tail == ".lnk") {
            target = target.substr(0, target.size() - 4);
        }
    }

    std::vector<fs::path> dirs = get_common_dirs();
    std::vector<std::string> bad_keywords = { "uninstall", "plugin", "capture", "temp", "update" };

    for (const auto& base : dirs) {
        try {
            for (const auto& entry : fs::recursive_directory_iterator(base, fs::directory_options::skip_permission_denied)) {
                if (!entry.is_regular_file()) continue;

                std::string ext = to_lower(entry.path().extension().string());
                std::string stem = to_lower(entry.path().stem().string());
                std::string full = to_lower(entry.path().string());

                if ((ext == ".exe" || ext == ".lnk") &&
                    stem.find(target) != std::string::npos) {

                    bool is_bad = false;
                    for (const auto& kw : bad_keywords) {
                        if (full.find(kw) != std::string::npos) {
                            is_bad = true;
                            break;
                        }
                    }
                    if (is_bad) continue;

                    // Prioritize exact stem match for .exe files
                    if (stem == target && ext == ".exe") {
                        return entry.path().string();
                    }

                    return entry.path().string();
                }
            }
        } catch (...) {
            continue;
        }
    }

    return "";
}

// Main function to start a process
std::string start_process(const std::string& raw_input) {
    std::string app_name = raw_input;
    app_name.erase(0, app_name.find_first_not_of(" \t\r\n"));
    app_name.erase(app_name.find_last_not_of(" \t\r\n") + 1);

    if (app_name.empty()) {
        return "❌ No process name provided.";
    }

    // Try launching directly via PATH
    HINSTANCE result = ShellExecuteA(NULL, "open", app_name.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)result > 32) {
        return "✅ Started (from PATH): " + app_name;
    }

    // If not found, search in common directories
    std::string guessed_path = find_app_path(app_name);
    if (guessed_path.empty()) {
        // Try SearchPath as a fallback
        char buffer[MAX_PATH];
        if (SearchPathA(NULL, app_name.c_str(), ".exe", MAX_PATH, buffer, NULL)) {
            guessed_path = buffer;
        } else {
            return "❌ Process not found: " + app_name;
        }
    }

    // Launch the found process
    result = ShellExecuteA(NULL, "open", guessed_path.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)result > 32) {
        return "✅ Started: " + fs::path(guessed_path).filename().string();
    }

    return "❌ Failed to launch: " + guessed_path;
}