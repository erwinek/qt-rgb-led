#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <signal.h>
#include <string>
#include "SerialReader.h"
#include "GifPlayer.h"

using namespace rgb_matrix;

std::atomic<bool> play_gif(false);
std::unique_ptr<GifPlayer> gif_player;

volatile bool interrupt_received = false;

static void InterruptHandler(int signo) {
    interrupt_received = true;
}

class LedTextDisplay {
public:
    LedTextDisplay()
        : score_(888), high_score_(600), credits_(0), scroll_text_("Boxer ProGames") {}

    void setScore(int score) { score_ = score; }
    void setHighScore(int high_score) { high_score_ = high_score; }
    void setCredits(int credits) { credits_ = credits; }
    void setScrollText(const std::string& text) { scroll_text_ = text; }

    void render(FrameCanvas* canvas, Font& font) {
        Color color1(255, 0, 0);    // czerwony
        Color color2(0, 255, 0);    // zielony
        Color color3(0, 0, 255);    // niebieski
        Color color4(255, 255, 0);  // żółty

        // Czyszczenie całego ekranu
        canvas->Clear();

        // Statyczne linie
        DrawText(canvas, font, 192/3, 40, color1, std::to_string(score_).c_str());
        DrawText(canvas, font, 192/3, 84, color2, std::to_string(high_score_).c_str());
        DrawText(canvas, font, 192/3, 128, color3, std::to_string(credits_).c_str());

        // Przewijający się tekst
        static int scroll_pos = canvas->width();
        int text_width = font.CharacterWidth('0') * scroll_text_.size(); // przybliżona szerokość
        scroll_pos -= 1;
        if (scroll_pos < -text_width) scroll_pos = canvas->width();
        DrawText(canvas, font, scroll_pos, 170, color4, scroll_text_.c_str());

    }

private:
    int score_;
    int high_score_;
    int credits_;
    std::string scroll_text_;
};

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    RuntimeOptions runtime_opt;

    matrix_options.rows = 64;
    matrix_options.cols = 64;
    matrix_options.chain_length = 3; // 64*3 = 192 px
    matrix_options.parallel = 3;     // 64*3 = 192 px
    matrix_options.hardware_mapping = "regular";
    runtime_opt.gpio_slowdown = 3;

    RGBMatrix* matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (!matrix) return 1;

    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    FrameCanvas* canvas = matrix->CreateFrameCanvas();

    Font font;
    if (!font.LoadFont("./fonts/ComicNeue-Bold-48.bdf")) {  // upewnij się, że font jest w katalogu fonts
        printf("Nie można wczytać fontu\n");
        return 1;
    }

    LedTextDisplay display;
	static int i = 0;

	SerialReader serial("/dev/ttyUSB0", B1000000);
	serial.setCommandHandler([&](const std::string& cmd) {
    if (cmd.rfind("SCORE", 0) == 0) {
        int val = std::stoi(cmd.substr(6));
        display.setScore(val);
    } else if (cmd.rfind("HISCORE", 0) == 0) {
        int val = std::stoi(cmd.substr(8));
        display.setHighScore(val);
    } else if (cmd.rfind("CREDITS", 0) == 0) {
        int val = std::stoi(cmd.substr(8));
        display.setCredits(val);
    } else if (cmd.rfind("TEXT", 0) == 0) {
        display.setScrollText(cmd.substr(5));
    }
});
serial.start();


	std::string path = "anime/cube-14564_256.gif";
        gif_player = std::make_unique<GifPlayer>(path);
	gif_player.load();
        play_gif = true;

    while (!interrupt_received) {
//        display.render(canvas, font);
//        canvas = matrix->SwapOnVSync(canvas);


        if (play_gif && gif_player) {
            gif_player->render(canvas);
        } else {
            display.render(canvas, font);
        }
        canvas = matrix->SwapOnVSync(canvas);


        usleep(30 * 1000); // ~33 FPS
    }

    matrix->Clear();
    delete matrix;
    return 0;
}
