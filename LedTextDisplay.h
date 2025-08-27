#ifndef LEDTEXTDISPLAY_H
#define LEDTEXTDISPLAY_H

#include <QImage>
#include <QPainter>
#include <QString>
#include <QFont>

class LedTextDisplay {
public:
    LedTextDisplay(int width, int height)
        : w(width), h(height),
          score(0), highScore(0), credits(0),
          scrollText(""), scrollX(width),
          font("Monospace", 10, QFont::Bold)
    {
        font.setStyleHint(QFont::TypeWriter);
    }

    void setScore(int v) { score = v; }
    void setHighScore(int v) { highScore = v; }
    void setCredits(int v) { credits = v; }
    void setScrollText(const QString &text) {
        scrollText = text;
        scrollX = w;
    }

    void render(QImage &image, QPainter &painter) {
        painter.setPen(Qt::green);
        painter.setFont(font);

        painter.drawText(1, 12, QString("%1").arg(score, 3, 10, QChar('0')));
        painter.drawText(1, 26, QString("%1").arg(highScore, 3, 10, QChar('0')));
        painter.drawText(1, 40, QString("%1").arg(credits, 2, 10, QChar('0')));

        painter.drawText(scrollX, 54, scrollText);
        int textWidth = painter.fontMetrics().horizontalAdvance(scrollText);
        scrollX = (scrollX < -textWidth) ? w : scrollX - 1;
    }

private:
    int w, h;
    int score, highScore, credits;
    QString scrollText;
    int scrollX;
    QFont font;
};

#endif // LEDTEXTDISPLAY_H
