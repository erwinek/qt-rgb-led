#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <led-matrix.h>

class GifPlayer {
public:
    GifPlayer(const std::string &path);
    bool load();  // ładuje klatki z pliku
    void render(rgb_matrix::FrameCanvas *canvas);

private:
    std::string path_;  // ścieżka do pliku
    int width_ = 0;
    int height_ = 0;

    std::vector<std::vector<uint8_t>> frames_;  // klatki RGB
    std::vector<int> delays_;                   // opóźnienia w ms
    size_t currentFrame_ = 0;                   // aktualna klatka
    uint64_t lastFrameTime_ = 0;                // czas ostatniej zmiany klatki
};
