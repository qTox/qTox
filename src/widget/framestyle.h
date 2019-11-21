#ifndef FRAMESTYLE_H
#define FRAMESTYLE_H

#include <QFrame>
#include "src/widget/gui.h"

class FrameStyle : public QFrame {
    Q_OBJECT

public:
    FrameStyle(QWidget* parent = nullptr) : QFrame(parent)
    {
        connect(&GUI::getInstance(), &GUI::themeReload, this, &FrameStyle::reloadTheme);
    }
    virtual ~FrameStyle() {}

public slots:
    virtual void reloadTheme() {}
};

#endif //FRAMESTYLE_H
