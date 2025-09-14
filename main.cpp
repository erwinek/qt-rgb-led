#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <signal.h>
#include <string>
#include "SerialReader.h"
#include "GifPlayer.h"
#include <iostream>
#include <stdexcept>
#include <chrono>

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
        : score_(888), high_score_(600), credits_(0), scroll_text_(""), m_Text1(""), m_Text2("") {
            if (!small_font.LoadFont("./fonts/7x13.bdf")) printf("Nie można wczytać small_font\n");
	    if (!medium_font.LoadFont("./fonts/9x18B.bdf")) printf("Nie można wczytać medium_font\n");
        }

    void setScore(int score) { 
	if (score < 1000 && score != 888) score_ = score; 
    }
    void setHighScore(int high_score) { 
	if (high_score<1000 && high_score != 888) high_score_ = high_score;
    }
    void setCredits(int credits) { credits_ = credits; }
    void setScrollText(const std::string& text) { scroll_text_ = text; }

    void render(FrameCanvas* canvas, Font& font) {
        Color color1(255, 0, 0);    // czerwony
        Color color2(0, 255, 0);    // zielony
        Color color3(0, 0, 255);    // niebieski
        Color color4(255, 255, 0);  // żółty
        Color colorBlack(0, 0, 0);  // czarny

        // Czyszczenie całego ekranu
        canvas->Clear();

        // Statyczne linie
        DrawText(canvas, small_font, 10, 26, color1, "SCORE:");
        DrawText(canvas, font, 192/3, 40, color1, std::to_string(score_).c_str());
        DrawText(canvas, small_font, 10, 72, color2, "RECORD:");
        DrawText(canvas, font, 192/3, 84, color2, std::to_string(high_score_).c_str());
        DrawText(canvas, small_font, 10, 115, color3, "Credit:");
	if(credits_==55) DrawText(canvas, small_font, 192/3, 115, color3, "FreePlay");
        else DrawText(canvas, medium_font, 192/3 + 33, 115, color3, std::to_string(credits_).c_str());

	//Przewijajacy sie text
	if(scroll_text_.length() > 0) {
            static int scroll_pos = canvas->width();
            int text_width = font.CharacterWidth('0') * scroll_text_.size(); // przybliżona szerokość
            scroll_pos -= 1;
            if (scroll_pos < -text_width) scroll_pos = canvas->width();
            DrawText(canvas, font, scroll_pos, 175, color4, scroll_text_.c_str());
	}
    else {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_blink).count();
        
        if (elapsed >= BLINK_INTERVAL) {
            text_visible = !text_visible;
            last_blink = now;
        }
        
        if (text_visible) {
            if(m_Text1.length() > 0) {
                const char* text = m_Text1.c_str();
                int x = 0;
                int text_width = DrawText(canvas, medium_font, x, 160, colorBlack, text);
                x = (192 - text_width) / 2;
                DrawText(canvas, medium_font, x, 160, color4, text);
            }      
            if(m_Text2.length() > 0) {
                const char* text = m_Text2.c_str();
                int x = 0;
                int text_width = DrawText(canvas, medium_font, x, 185, colorBlack, text);
                x = (192 - text_width) / 2;
                DrawText(canvas, medium_font, x, 175, color4, text);
            }  
        }
    }
}

    void setText1(const std::string& text) { m_Text1 = text; }
    void setText2(const std::string& text) { m_Text2 = text; }


private:
    int score_;
    int high_score_;
    int credits_;
    std::string scroll_text_;
    std::string m_Text1;
    std::string m_Text2;
    Font small_font;
    Font medium_font;
    bool text_visible = true;
    std::chrono::steady_clock::time_point last_blink = std::chrono::steady_clock::now();
    const int BLINK_INTERVAL = 300; // ms
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
    auto safe_stoi = [&](const std::string& s, int &out) -> bool {
        try {
            out = std::stoi(s);
            return true;
        } catch (const std::invalid_argument&) {
            std::cerr << "Błąd: nieprawidłowa liczba: '" << s << "' dla komendy: " << cmd << "\n";
        } catch (const std::out_of_range&) {
            std::cerr << "Błąd: liczba poza zakresem: '" << s << "' dla komendy: " << cmd << "\n";
        }
        return false;
    };

    if (cmd.rfind("SCORE", 0) == 0) {
        int val;
        std::string arg = cmd.substr(6);
        if (safe_stoi(arg, val)) display.setScore(val);

    } else if (cmd.rfind("HISCORE", 0) == 0) {
        int val;
        std::string arg = cmd.substr(8);
        if (safe_stoi(arg, val)) display.setHighScore(val);

    } else if (cmd.rfind("CREDITS", 0) == 0) {
        int val;
        std::string arg = cmd.substr(8);
        if (safe_stoi(arg, val)) display.setCredits(val);

    } else if (cmd.rfind("TEXT1", 0) == 0) {
        std::string arg = cmd.substr(6);
        display.setText1(arg);

    } else if (cmd.rfind("TEXT2", 0) == 0) {
        std::string arg = cmd.substr(6);
        display.setText2(arg);

    } else if (cmd.rfind("SCROLL", 0) == 0) {
        std::string arg = cmd.substr(7);
        display.setScrollText(arg);
    } else if (cmd.rfind("STOP_GIF", 0) == 0) {
        play_gif = false;

    } else if (cmd.rfind("PLAY_GIF", 0) == 0) {
        std::string name = cmd.substr(9);
        if (!name.empty()) {
            std::string path = "anime/" + name + ".gif";
            
            // Sprawdź czy ten sam GIF jest już załadowany
            if (gif_player && gif_player->getCurrentPath() == path) {
                play_gif = true;  // Tylko włącz wyświetlanie
                return;
            }

            std::cout << "Ładowanie GIF: " << path << std::endl;
            
            if (!gif_player) {
                gif_player = std::make_unique<GifPlayer>();
            }
            
            if (gif_player->load(path)) {
                play_gif = true;
            } else {
                std::cerr << "Błąd: nie udało się załadować GIF: " << path << "\n";
                play_gif = false;
            }
        } else {
            std::cerr << "Błąd: brak nazwy pliku w PLAY_GIF\n";
        }
    }
});





serial.start();

    while (!interrupt_received) {

        if (play_gif && gif_player) {
            gif_player->render(canvas);
        } else {
            display.render(canvas, font);
        }
        canvas = matrix->SwapOnVSync(canvas);


        usleep(40 * 1000); // ~33 FPS
    }

    matrix->Clear();
    delete matrix;
    return 0;
}
