#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QFont>
#include <thread>
#include <chrono> // do std::this_thread::sleep_for

#include "led-matrix.h"
#include "graphics.h"

using namespace rgb_matrix;

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv); // headless, nie tworzy okna

    // Konfiguracja matrycy LED
    RGBMatrix::Options matrix_options;
    matrix_options.rows = 64;
    matrix_options.cols = 64;
    matrix_options.chain_length = 1;
    matrix_options.parallel = 3;   // Active-3
    matrix_options.hardware_mapping = "regular";

    RuntimeOptions runtime_opt;
    runtime_opt.gpio_slowdown = 2; // Zero 2 W

    RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, runtime_opt);
    if (!matrix) {
        fprintf(stderr, "Nie udało się zainicjalizować matrycy!\n");
        return 1;
    }

    FrameCanvas *canvas = matrix->CreateFrameCanvas();

    // Tworzymy obraz w QImage
    QImage img(64, 64, QImage::Format_RGB888);
    img.fill(Qt::black);

    QPainter painter(&img);
    painter.setPen(Qt::white);
    QFont font("Arial", 14, QFont::Bold);
    painter.setFont(font);
    painter.drawText(img.rect(), Qt::AlignCenter, QString("Hello LED!"));
    painter.end();

    // Przekopiowanie pikseli QImage -> LED
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QColor c = img.pixelColor(x, y);
            canvas->SetPixel(x, y, c.red(), c.green(), c.blue());
        }
    }

    canvas = matrix->SwapOnVSync(canvas);

    // Poczekaj kilka sekund, żeby zobaczyć efekt
    std::this_thread::sleep_for(std::chrono::seconds(5));

    delete matrix;
    return 0;
}
