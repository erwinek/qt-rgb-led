#include "led-matrix.h"
#include "graphics.h"

#include <iostream>
#include <signal.h>
#include <string>
#include <thread>
#include <chrono>

using namespace rgb_matrix;

volatile bool interrupt_received = false;

static void InterruptHandler(int signo) {
    interrupt_received = true;
}

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    RuntimeOptions runtime_opt;

    matrix_options.rows = 64;
    matrix_options.cols = 64;
    matrix_options.chain_length = 3;
    matrix_options.parallel = 3;
    matrix_options.hardware_mapping = "regular";
    runtime_opt.gpio_slowdown = 3;

    RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (!matrix) return 1;

    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    FrameCanvas *canvas = matrix->CreateFrameCanvas();

    Font font;
    if (!font.LoadFont("fonts/7x13.bdf")) {
        std::cerr << "Nie można wczytać fontu\n";
        return 1;
    }

    Color text_color(0, 255, 0); // zielony

    std::string scroll_text = "Boxer ProGames";
    int scroll_pos = canvas->width();

    int score = 888;
    int high_score = 600;
    int credits = 99;

    while (!interrupt_received) {
        canvas->Clear();


DrawText(canvas, font, 1, 12, text_color, std::to_string(score).c_str());
DrawText(canvas, font, 1, 26, text_color, std::to_string(high_score).c_str());
DrawText(canvas, font, 1, 40, text_color, std::to_string(credits).c_str());
DrawText(canvas, font, scroll_pos, 54, text_color, scroll_text.c_str());


        // Linia 1: Wynik

        // Linia 2: Rekord

        // Linia 3: Kredyty

        // Linia 4: przewijany tekst
        scroll_pos--;
        if (scroll_pos < -int(scroll_text.length() * 7)) scroll_pos = canvas->width(); // 7 to szerokość znaku 7x13

        canvas = matrix->SwapOnVSync(canvas);
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // ~20 FPS
    }

    matrix->Clear();
    delete matrix;
    return 0;
}
