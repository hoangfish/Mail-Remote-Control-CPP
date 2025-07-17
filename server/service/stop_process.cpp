#include "../../lib/service.h"
#include <windows.h>
#include <sstream>
#include <tlhelp32.h>
#include <cstdlib>
#include <algorithm>
string stop_process(const string &pid_str)
{
    if (pid_str.empty() || !std::all_of(pid_str.begin(), pid_str.end(), ::isdigit))
    {
        return "Invalid PID: " + pid_str;
    }

    int pid = std::stoi(pid_str);

#ifdef _WIN32
    std::ostringstream command;
    command << "taskkill /PID " << pid << " /F";
    FILE *pipe = _popen(command.str().c_str(), "r");
    if (!pipe)
        return "Failed to run taskkill.";

    char buffer[128];
    string result;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        result += buffer;
    }
    _pclose(pipe);

    if (result.find("SUCCESS") != string::npos || result.find("terminated") != string::npos)
    {
        return "Stopped process " + std::to_string(pid) + ".";
    }
    else
    {
        return "Failed to stop process " + std::to_string(pid) + ": " + result;
    }
#else
    // For Linux / macOS
    int ret = kill(pid, SIGKILL);
    if (ret == 0)
    {
        return "Stopped process " + std::to_string(pid) + ".";
    }
    else
    {
        if (errno == ESRCH)
            return "Process does not exist: " + std::to_string(pid);
        if (errno == EPERM)
            return "Permission denied to stop: " + std::to_string(pid);
        return "Failed to stop process: " + std::to_string(pid);
    }
#endif
}
