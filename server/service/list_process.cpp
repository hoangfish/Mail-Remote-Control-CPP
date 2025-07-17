#include "service.h"
#include <string>
#include <vector>
#include <tuple>
#include <sstream>
#include <array>
#include <cstdio>
#include <algorithm>
#include "json.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using std::string;
using std::vector;
using std::array;
using json = nlohmann::json;

vector<ProcessInfo> list_processes() {
    vector<ProcessInfo> processes;
    array<char, 1024> buffer;

#ifdef _WIN32
    string command = "tasklist /fo csv /nh";
    FILE* pipe = _popen(command.c_str(), "r");
#else
    string command = "ps -eo pid,user,comm";
    FILE* pipe = popen(command.c_str(), "r");
#endif

    if (!pipe) return processes;

    bool firstLine = true;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        string line = buffer.data();

#ifdef _WIN32
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        vector<string> fields;
        size_t start = 0;
        while (start < line.size()) {
            if (line[start] == '"') {
                size_t end = line.find('"', start + 1);
                if (end == string::npos) break;
                fields.push_back(line.substr(start + 1, end - start - 1));
                start = line.find(',', end);
                if (start == string::npos) break;
                ++start;
            } else {
                size_t end = line.find(',', start);
                fields.push_back(line.substr(start, end - start));
                if (end == string::npos) break;
                start = end + 1;
            }
        }

        if (fields.size() >= 2) {
            ProcessInfo info;
            info.name = fields[0];
            info.pid = fields[1];
            info.user = "-";
            processes.push_back(info);
        }
#else
        if (firstLine) {
            firstLine = false;
            continue;
        }

        std::istringstream iss(line);
        string pid, user;
        if (!(iss >> pid >> user)) continue;

        string cmd;
        getline(iss, cmd);
        size_t pos = cmd.find_first_not_of(" \t");
        if (pos != string::npos) {
            cmd = cmd.substr(pos);
        }

        processes.push_back({pid, cmd, user});
#endif
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return processes;
}

string list_processes_json() {
    vector<ProcessInfo> processes = list_processes();
    json arr = json::array();

    for (const auto& p : processes) {
        arr.push_back({
            {"pid", p.pid},
            {"name", p.name},
            {"user", p.user}
        });
    }

    json wrapper;
    wrapper["processes"] = arr;
    return wrapper.dump();
}
