cmake_minimum_required(VERSION 3.10)
project(qt-rgb-led)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Źródła
set(SOURCES
    main.cpp
    SerialReader.cpp
    GifPlayer.cpp
    external/gifdec.c
)

set(HEADERS
    SerialReader.h
    GifPlayer.h
)

# Tworzymy executable
add_executable(qt-rgb-led ${SOURCES} ${HEADERS})

# Ścieżki do nagłówków
target_include_directories(qt-rgb-led PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external
    /home/erwinek/rpi-rgb-led-matrix/include
)

# Linkowanie biblioteki RGB LED Matrix
target_link_libraries(qt-rgb-led
    /home/erwinek/rpi-rgb-led-matrix/lib/librgbmatrix.a
    pthread
    rt
    m
)
