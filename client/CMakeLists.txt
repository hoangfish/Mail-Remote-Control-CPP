cmake_minimum_required(VERSION 3.16)

project(MyQtProject LANGUAGES CXX)

# =====================
# C++ Standard
# =====================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =====================
# Qt settings
# =====================
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# =====================
# OpenSSL & CURL setup (fix not found error)
# =====================
# Adjust this path depending on your MSYS2 environment:
# If you use ucrt64: replace "mingw64" with "ucrt64"
set(OPENSSL_ROOT_DIR "C:/msys64/mingw64")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(ZLIB_ROOT "C:/msys64/mingw64")
set(ZLIB_INCLUDE_DIR "${ZLIB_ROOT}/include")
set(ZLIB_LIBRARY "${ZLIB_ROOT}/lib/libz.dll.a")

find_package(CURL REQUIRED)

# =====================
# Include paths
# =====================
include_directories(../lib)

# =====================
# Source files
# =====================
set(SOURCES
    main.cpp
    MainWindow_ui.cpp
    MailRemote_ui.cpp
    CustomMode_ui.cpp
    Download_file.cpp
    AboutUs_ui.cpp
    CustomMode.cpp
    Capture_photo.cpp
    Record_video.cpp
    MailRemote.cpp
    utils.cpp
    gmailapi.cpp
    mail_reader.cpp
)

# =====================
# Header files
# =====================
set(HEADERS
    ../lib/MainWindow_ui.h
    ../lib/MailRemote_ui.h
    ../lib/CustomMode_ui.h
    ../lib/AboutUs_ui.h
    ../lib/Download_file.h
    ../lib/CustomMode.h
    ../lib/Capture_photo.h
    ../lib/Record_video.h
    ../lib/gmailapi.h
    ../lib/Mail_reader.h
    ../lib/MailRemote.h
    ../lib/utils.h
    ../lib/curl.h
    ../lib/curlver.h
    ../lib/easy.h
    ../lib/header.h
    ../lib/mprintf.h
    ../lib/multi.h
    ../lib/options.h
    ../lib/stdcheaders.h
    ../lib/system.h
    ../lib/typecheck-gcc.h
    ../lib/urlapi.h
    ../lib/websockets.h
)

# =====================
# Add executable
# =====================
add_executable(MyQtApp ${SOURCES} ${HEADERS})

# =====================
# Copy JSON file
# =====================
configure_file(credentials.json credentials.json COPYONLY)

# =====================
# Link libraries
# =====================
target_link_libraries(MyQtApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    CURL::libcurl
)

# =====================
# Copy folders
# =====================
file(COPY images DESTINATION ${CMAKE_BINARY_DIR})
