cmake_minimum_required(VERSION 3.5)

project(qt-rgb-led LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Gui REQUIRED)

# 🔹 Ścieżka do repozytorium hzeller/rpi-rgb-led-matrix
set(RGBMATRIX_DIR "/home/erwinek/rpi-rgb-led-matrix")

include_directories(${RGBMATRIX_DIR}/include)
link_directories(${RGBMATRIX_DIR}/lib)

add_executable(qt-rgb-led main.cpp)

target_link_libraries(qt-rgb-led
    Qt5::Gui
    rgbmatrix
    pthread
    rt
    m
)
