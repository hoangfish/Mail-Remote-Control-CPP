#include "../../lib/service.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <iostream>

using namespace std;
using namespace cv;

void capture_photo(SOCKET conn) {
    // Thiết lập socket options
    int bufferSize = 131072; // 128KB
    setsockopt(conn, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&bufferSize), sizeof(bufferSize));
    int flag = 1;
    setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flag), sizeof(flag));

    // Lấy handle của màn hình
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);

    // Chụp màn hình
    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);
    cout << "Captured screenshot: " << screenWidth << "x" << screenHeight << endl;

    // Chuyển bitmap thành OpenCV Mat
    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screenWidth;
    bi.biHeight = -screenHeight; // Âm để đảo ngược (top-down)
    bi.biPlanes = 1;
    bi.biBitCount = 24; // RGB
    bi.biCompression = BI_RGB;

    Mat frame(screenHeight, screenWidth, CV_8UC3);
    GetDIBits(hMemoryDC, hBitmap, 0, screenHeight, frame.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // Giải phóng tài nguyên GDI
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    if (frame.empty()) {
        string error = "__ERROR__Failed to capture screenshot.";
        send(conn, error.c_str(), error.size(), 0);
        cout << "Error: Failed to capture screenshot." << endl;
        return;
    }

    // Encode ảnh thành định dạng JPG trong RAM
    vector<uchar> encoded;
    vector<int> params = { IMWRITE_JPEG_QUALITY, 90 };
    if (!imencode(".jpg", frame, encoded, params)) {
        string error = "__ERROR__Failed to encode image.";
        send(conn, error.c_str(), error.size(), 0);
        cout << "Error: Failed to encode image." << endl;
        return;
    }
    cout << "Encoded image size: " << encoded.size() << " bytes" << endl;

    if (encoded.empty()) {
        string error = "__ERROR__Empty encoded image.";
        send(conn, error.c_str(), error.size(), 0);
        cout << "Error: Empty encoded image." << endl;
        return;
    }

    // Gửi header nhận diện
    string header = "__PHOTO_";
    int sent = send(conn, header.c_str(), header.size(), 0);
    if (sent == SOCKET_ERROR) {
        cout << "Error sending header: " << WSAGetLastError() << endl;
        return;
    }
    cout << "Sent header: " << header << ", bytes sent: " << sent << endl;

    // Gửi kích thước ảnh (4 byte)
    uint32_t imgSize = static_cast<uint32_t>(encoded.size());
    sent = send(conn, reinterpret_cast<char*>(&imgSize), sizeof(imgSize), 0);
    if (sent == SOCKET_ERROR) {
        cout << "Error sending image size: " << WSAGetLastError() << endl;
        return;
    }
    cout << "Sent image size: " << imgSize << ", bytes sent: " << sent << endl;

    // Gửi toàn bộ ảnh, chia thành các chunk nhỏ
    size_t totalSent = 0;
    const char* data = reinterpret_cast<char*>(encoded.data());
    const int chunkSize = 8192;
    while (totalSent < imgSize) {
        int remaining = static_cast<int>(min(static_cast<size_t>(chunkSize), imgSize - totalSent));
        sent = send(conn, data + totalSent, remaining, 0);
        if (sent == SOCKET_ERROR) {
            cout << "Error sending image data: " << WSAGetLastError() << endl;
            return;
        }
        totalSent += sent;
        Sleep(10); // Ngủ ngắn để tránh chặn socket
    }
    cout << "Sent all image data: " << totalSent << " bytes" << endl;
}