#include "../../lib/service.h"
#include <windows.h>
#include <sstream>

string start_process(const string& command_line) {
    if (command_line.empty()) {
        return "No command given.";
    }

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring wcmd(command_line.begin(), command_line.end());
    wchar_t* cmd = &wcmd[0];

    if (!CreateProcessW(
            NULL,
            cmd,
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi)) {
        std::ostringstream oss;
        oss << "Error starting process. Error code: " << GetLastError();
        return oss.str();
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return "Started process: " + command_line;
}
