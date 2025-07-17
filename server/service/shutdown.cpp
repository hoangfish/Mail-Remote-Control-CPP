#include "../../lib/service.h"
#include <cstdlib>
#include <string>

std::string shutdown_system()
{
#ifdef _WIN32
    int result = system("shutdown /s /t 0");
#elif defined(__linux__) || defined(__APPLE__)
    int result = system("sudo shutdown now");
#else
    return "Shutdown: Unsupported OS.";
#endif

    if (result == 0)
    {
        return "Shutdown: Command sent to system.";
    }
    else
    {
        return "Shutdown: Failed to execute command.";
    }
}
