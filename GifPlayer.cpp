#include "GifPlayer.h"
#include "gifdec.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <algorithm> // std::min

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
            memset(frame, 0, gif->width * gif->height * 3); // czarne tło
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
    if (frames_.empty() || currentFrame_ >= frames_.size() ||
        delays_.empty() || delays_.size() != frames_.size()) return;

    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    if (lastFrameTime_ == 0) lastFrameTime_ = now;
    if (now - lastFrameTime_ >= (uint64_t)delays_[currentFrame_]) {
        currentFrame_ = (currentFrame_ + 1) % frames_.size();
        lastFrameTime_ = now;
    }

    const auto &frame = frames_[currentFrame_];

    int cw = canvas->width();
    int ch = canvas->height();

    int draw_w = std::min(width_,  cw);
    int draw_h = std::min(height_, ch);

    int off_x = (cw - draw_w) / 2;
    int off_y = (ch - draw_h) / 2;

    int idx = 0;
    for (int y = 0; y < height_; ++y) {
        // jeśli nie mieści się w canvas, tylko pomiń wiersz
        if (y >= draw_h) {
            idx += width_ * 3;
            continue;
        }

        for (int x = 0; x < width_; ++x) {
            if (idx + 2 >= (int)frame.size()) {
                // bezpieczeństwo: wyjdź z pętli jeśli GIF jest mniejszy niż deklarowane wymiary
                return;
            }

            uint8_t r = frame[idx++];
            uint8_t g = frame[idx++];
            uint8_t b = frame[idx++];

            if (x < draw_w) {
                int draw_x = off_x + x;
                int draw_y = off_y + y;
                if (draw_x >= 0 && draw_x < cw &&
                    draw_y >= 0 && draw_y < ch) {
                    canvas->SetPixel(draw_x, draw_y, r, g, b);
                }
            }
        }
    }
}
