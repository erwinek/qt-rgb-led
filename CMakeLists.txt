cmake_minimum_required(VERSION 3.10)
project(qt-rgb-led CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dodaj pliki źródłowe
set(SOURCES
    main.cpp
    SerialReader.cpp
)

set(HEADERS
    SerialReader.h
)

# Główna biblioteka LED-matrix
include_directories(/home/erwinek/rpi-rgb-led-matrix/include)
link_directories(/home/erwinek/rpi-rgb-led-matrix/lib)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    rgbmatrix
    pthread
)
