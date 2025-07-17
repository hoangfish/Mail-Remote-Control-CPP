cmake_minimum_required(VERSION 3.16)
project(Server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# ABI fix nếu cần
add_compile_options(-D_GLIBCXX_USE_CXX11_ABI=1)

# Find OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/../lib)

# Gộp tất cả .cpp trong thư mục service/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/service/*.cpp
)

add_executable(server ${SOURCES})

# Link các thư viện cần thiết
target_link_libraries(server PRIVATE
    ws2_32
    opencv_core
    opencv_imgproc
    opencv_imgcodecs
    opencv_videoio
    opencv_highgui
)

# 🔧 Link tĩnh C++ để tránh lỗi DLL
target_link_options(server PRIVATE -static-libstdc++ -static-libgcc)