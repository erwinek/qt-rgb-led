#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <signal.h>
#include <string>

using namespace rgb_matrix;

volatile bool interrupt_received = false;

static void InterruptHandler(int signo) {
    interrupt_received = true;
}

class LedTextDisplay {
public:
    LedTextDisplay(int width, int height)
        : width_(width), height_(height),
          score_(888), high_score_(600), credits_(99),
          scroll_text_("Boxer ProGames") {}

    void setScore(int score) { score_ = score; }
    void setHighScore(int high_score) { high_score_ = high_score; }
    void setCredits(int credits) { credits_ = credits; }
    void setScrollText(const std::string& text) { scroll_text_ = text; }

    void render(FrameCanvas* canvas, Font& font) {
        Color color1(255, 0, 0);     // czerwony
        Color color2(0, 255, 0);     // zielony
        Color color3(0, 0, 255);     // niebieski
        Color color4(255, 255, 0);   // żółty

        // Wiersz 1: Wynik
        DrawText(canvas, font, 1, 30, color1, std::to_string(score_).c_str());

        // Wiersz 2: Rekord
        DrawText(canvas, font, 1, 60, color2, std::to_string(high_score_).c_str());

        // Wiersz 3: Kredyty
        DrawText(canvas, font, 1, 90, color3, std::to_string(credits_).c_str());

        // Wiersz 4: Przewijany tekst
        static int scroll_pos = width_;
        DrawText(canvas, font, scroll_pos, 150, color4, scroll_text_.c_str());
        scroll_pos--;
        if (scroll_pos < -int(scroll_text_.length() * font.CharacterWidth('A'))) // przybliżona szerokość
            scroll_pos = width_;
    }

private:
    int width_;
    int height_;
    int score_;
    int high_score_;
    int credits_;
    std::string scroll_text_;
};

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    RuntimeOptions runtime_opt;

    // ustawienia matrycy 192x192 (3x3 64x64)
    matrix_options.rows = 64;
    matrix_options.cols = 64;
    matrix_options.chain_length = 3;
    matrix_options.parallel = 3;
    matrix_options.hardware_mapping = "regular";
    runtime_opt.gpio_slowdown = 3;

    RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == nullptr)
        return 1;

    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    FrameCanvas *canvas = matrix->CreateFrameCanvas();

    Font font;
    if (!font.LoadFont("fonts/5x8.bdf")) {
        fprintf(stderr, "Nie można wczytać fontu\n");
        return 1;
    }

    LedTextDisplay display(matrix->width(), matrix->height());

    while (!interrupt_received) {
        canvas->Clear();
        display.render(canvas, font);
        canvas = matrix->SwapOnVSync(canvas);
        usleep(30 * 1000); // ~33 FPS
    }

    matrix->Clear();
    delete matrix;
    return 0;
}
