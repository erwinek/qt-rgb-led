#include "GifPlayer.h"

// Definicja implementacji STB w jednym pliku
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GifPlayer::GifPlayer(const std::string& path) {
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &width_, &height_, &channels, 3);
    if (data) {
        frames_.emplace_back(data, data + (width_ * height_ * 3));
        stbi_image_free(data);
    }
}

void GifPlayer::render(rgb_matrix::FrameCanvas* canvas) {
    if (frames_.empty()) return;

    const auto& frame = frames_[0];
    int idx = 0;
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            uint8_t r = frame[idx++];
            uint8_t g = frame[idx++];
            uint8_t b = frame[idx++];
            canvas->SetPixel(x, y, r, g, b);
        }
    }
}
