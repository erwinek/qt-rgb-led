#include "led-matrix.h"
#include "graphics.h"

#include <QImage>
#include <QPainter>
#include <QColor>
#include <QFont>
#include <QTime>

#include <unistd.h>
#include <signal.h>
#include <memory>
#include <iostream>

#include "LedTextDisplay.h"

using namespace rgb_matrix;
volatile bool interrupt_received = false;

static void InterruptHandler(int signo) {
    interrupt_received = true;
}

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    RuntimeOptions runtime_opt;

    // Ustawienia matrycy LED
    matrix_options.rows = 64;
    matrix_options.cols = 64;
    matrix_options.chain_length = 3;
    matrix_options.parallel = 3;
    matrix_options.hardware_mapping = "regular";
    runtime_opt.gpio_slowdown = 3;

    // Tworzymy matrycę
    RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == nullptr)
        return 1;

    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

    FrameCanvas *offscreen = matrix->CreateFrameCanvas();

    // Klasa wyświetlająca tekst
    LedTextDisplay display(matrix->width(), matrix->height());
    display.setScore(888);
    display.setHighScore(600);
    display.setCredits(99);
    display.setScrollText("Boxer ProGames");

    QImage image(matrix->width(), matrix->height(), QImage::Format_RGB32);
    QPainter painter;

    while (!interrupt_received) {
        image.fill(Qt::black);
        painter.begin(&image);

        // Renderujemy tekst
        display.render(image, painter);

        painter.end();

        // Kopiujemy obraz do matrycy LED
        for (int y = 0; y < matrix->height(); ++y) {
            for (int x = 0; x < matrix->width(); ++x) {
                QColor c = image.pixelColor(x, y);
                offscreen->SetPixel(x, y, c.red(), c.green(), c.blue());
            }
        }

        offscreen = matrix->SwapOnVSync(offscreen);

        usleep(30 * 1000); // ~33 FPS
    }

    matrix->Clear();
    delete matrix;
    return 0;
}
