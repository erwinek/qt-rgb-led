cmake_minimum_required(VERSION 3.5)
project(qt-rgb-led LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ścieżka do biblioteki rpi-rgb-led-matrix
set(RGBMATRIX_DIR "$ENV{HOME}/rpi-rgb-led-matrix")
include_directories(${RGBMATRIX_DIR}/include)
link_directories(${RGBMATRIX_DIR}/lib)

add_executable(qt-rgb-led main.cpp LedTextDisplay.h)

target_link_libraries(qt-rgb-led
    rgbmatrix
    pthread
)
