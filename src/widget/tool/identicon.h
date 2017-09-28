#ifndef IDENTICON_H
#define IDENTICON_H

#include <QColor>
#include <QImage>

class Identicon
{
public:
    Identicon(const QByteArray& data);
    QImage toImage(int scaleFactor = 1);

public:
    static const int IDENTICON_ROWS = 5;

private:
    float bytesToColor(QByteArray bytes);

private:
    static const int IDENTICON_COLOR_BYTES = 6;
    static const int ACTIVE_COLS = (IDENTICON_ROWS + 1) / 2;
    static const int COLORS = 2;
    static const int HASH_MIN_LEN = ACTIVE_COLS * IDENTICON_ROWS + COLORS * IDENTICON_COLOR_BYTES;

    uint8_t identiconColors[IDENTICON_ROWS][ACTIVE_COLS];
    QColor colors[COLORS];
};

#endif // IDENTICON_H
