#pragma once
#include <string>
#include <vector>
#include <led-matrix.h>   // <-- tego brakowało
#include <graphics.h>

class GifPlayer {
public:
    GifPlayer(const std::string& path);
    void render(rgb_matrix::FrameCanvas* canvas);

private:
    std::vector<std::vector<uint8_t>> frames_; // proste przechowywanie klatek
    int width_;
    int height_;
};
