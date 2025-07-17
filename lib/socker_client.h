#pragma once

#include <string>

// Gửi lệnh tới server và nhận phản hồi
std::string send_command_to_server(const std::string& ip, const std::string& command, const std::string& value = "");
