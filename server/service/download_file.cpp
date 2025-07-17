#include "../../lib/service.h"
#include <winsock2.h>
#include <fstream>
#include <filesystem>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

void download_file(SOCKET conn, const std::string& path) {
    namespace fs = std::filesystem;

    if (path.empty() || !fs::exists(path) || !fs::is_regular_file(path)) {
        std::string error = "__ERROR__File not found or invalid path";
        send(conn, error.c_str(), error.size(), 0);
        return;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::string error = "__ERROR__Failed to open file";
        send(conn, error.c_str(), error.size(), 0);
        return;
    }

    // Đọc file và gửi
    std::string header = "__FILE__";
    send(conn, header.c_str(), header.size(), 0);

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        send(conn, buffer, file.gcount(), 0);
    }
}
