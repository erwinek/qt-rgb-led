#include "GifPlayer.h"
#include "gifdec.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <algorithm> // std::min

GifPlayer::GifPlayer() {
}


bool GifPlayer::load(const std::string& new_filename) {
    if (!new_filename.empty()) {
        path_ = new_filename;
    }
    
    if (path_.empty()) {
        return false;
    }

    try {
        gd_GIF *gif = gd_open_gif(path_.c_str());
        if (!gif) {
            std::cerr << "Nie mogę otworzyć GIF: " << path_ << std::endl;
            return false;
        }

        int orig_width = gif->width;
        int orig_height = gif->height;
        
        // Oblicz współczynnik skalowania zachowując proporcje
        float scale_w = 192.0f / orig_width;
        float scale_h = 192.0f / orig_height;
        float scale = std::min(scale_w, scale_h);
        
        // Oblicz nowe wymiary
        width_ = static_cast<int>(orig_width * scale);
        height_ = static_cast<int>(orig_height * scale);
        
        // Oblicz offset do wycentrowania
        int offset_x = (192 - width_) / 2;
        int offset_y = (192 - height_) / 2;

        frames_.clear();
        delays_.clear();

        uint8_t *orig_frame = (uint8_t*)malloc(orig_width * orig_height * 3);
        std::vector<uint8_t> scaled_frame(192 * 192 * 3, 0); // Czarne tło

        if (!orig_frame) {
            gd_close_gif(gif);
            return false;
        }

        do {
            if (gd_get_frame(gif) == 1) {
                // Get frame data separately
                gd_render_frame(gif, orig_frame);
                
                // Skaluj każdą klatkę
                for (int y = 0; y < 192; y++) {
                    for (int x = 0; x < 192; x++) {
                        // Mapuj piksele ze skalowaniem
                        int src_x = (x - offset_x) / scale;
                        int src_y = (y - offset_y) / scale;
                        
                        if (src_x >= 0 && src_x < orig_width && 
                            src_y >= 0 && src_y < orig_height) {
                            // Kopiuj piksele z oryginalnej klatki
                            int src_idx = (src_y * orig_width + src_x) * 3;
                            int dst_idx = (y * 192 + x) * 3;
                            
                            scaled_frame[dst_idx] = orig_frame[src_idx];     // R
                            scaled_frame[dst_idx + 1] = orig_frame[src_idx + 1]; // G
                            scaled_frame[dst_idx + 2] = orig_frame[src_idx + 2]; // B
                        }
                    }
                }
                
                frames_.push_back(scaled_frame);
                delays_.push_back(gif->gce.delay * 10); // delay w ms
            }
        } while (gd_get_frame(gif) > 0);

        free(orig_frame);
        gd_close_gif(gif);

        currentFrame_ = 0;
        lastFrameTime_ = 0;

        return !frames_.empty();
    }
    catch(std::exception& e) {
        std::cerr << "Error loading GIF: " << e.what() << std::endl;
        return false;
    }
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
