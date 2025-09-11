#include "GifPlayer.h"
#include "gifdec.h"
#include <iostream>
#include <unistd.h>

GifPlayer::GifPlayer(const std::string &path) : path_(path) {}

bool GifPlayer::load() {
    gd_GIF *gif = gd_open_gif(path_.c_str());
    if (!gif) {
        std::cerr << "Nie mogę otworzyć GIF: " << path_ << std::endl;
        return false;
    }

    width_ = gif->width;
    height_ = gif->height;

    frames_.clear();
    delays_.clear();

    uint8_t *frame = (uint8_t*)malloc(gif->width * gif->height * 3);
    if (!frame) {
        gd_close_gif(gif);
        return false;
    }

    do {
        if (gd_get_frame(gif)) {
            gd_render_frame(gif, frame);

            // zapisz RGB
            std::vector<uint8_t> rgb(frame, frame + (gif->width * gif->height * 3));
            frames_.push_back(rgb);

            // delay w ms (gce.delay = setki ms)
            delays_.push_back(gif->gce.delay * 10);
        }
    } while (gd_get_frame(gif) > 0);

    free(frame);
    gd_close_gif(gif);

    currentFrame_ = 0;
    lastFrameTime_ = 0;

    return !frames_.empty();
}

void GifPlayer::render(rgb_matrix::FrameCanvas *canvas) {
    if (frames_.empty()) return;

    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch()
                   ).count();

    if (lastFrameTime_ == 0) {
        lastFrameTime_ = now;
    }

    if (now - lastFrameTime_ >= (uint64_t)delays_[currentFrame_]) {
        currentFrame_ = (currentFrame_ + 1) % frames_.size();
        lastFrameTime_ = now;
    }

    const auto &frame = frames_[currentFrame_];
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
