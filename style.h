#ifndef STYLE_H
#define STYLE_H

#include <QString>

class Style
{
public:
    static QString get(const QString& filename);
private:
    Style();
};

#endif // STYLE_H
