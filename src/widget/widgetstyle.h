#ifndef WIDGETSTYLE_H
#define WIDGETSTYLE_H

#include <QWidget>
#include "src/widget/gui.h"

class WidgetStyle : public QWidget {
    Q_OBJECT

public:
    WidgetStyle(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) : QWidget(parent, f)
    {
        connect(&GUI::getInstance(), &GUI::themeReload, this, &WidgetStyle::reloadTheme);
    }
    virtual ~WidgetStyle() {}

public slots:
    virtual void reloadTheme() {}
};

#endif //WIDGETSTYLE_H
