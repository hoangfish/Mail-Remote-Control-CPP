#include "../../lib/service.h"
#include <cstdlib>
#include <string>

std::string restart_system()
{
#ifdef _WIN32
    int result = system("shutdown /r /t 0");
#elif defined(__linux__) || defined(__APPLE__)
    int result = system("sudo reboot");
#else
    return "Restart: Unsupported OS.";
#endif

    if (result == 0)
    {
        return "Restart: Command sent to system.";
    }
    else
    {
        return "Restart: Failed to execute command.";
    }
}
