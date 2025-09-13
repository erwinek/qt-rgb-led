#pragma once
#include "led-matrix.h"
#include <string>
#include <vector>

class GifPlayer {
public:
    GifPlayer();
    bool load(const std::string& new_filename = "");
    void render(rgb_matrix::FrameCanvas* canvas);
    const std::string& getCurrentPath() const { return path_; }

private:
    std::string path_;
    int width_ = 0;
    int height_ = 0;
    std::vector<std::vector<uint8_t>> frames_;
    std::vector<int> delays_;
    size_t currentFrame_ = 0;
    uint64_t lastFrameTime_ = 0;
};
