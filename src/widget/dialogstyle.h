#ifndef DIALOGSTYLE_H
#define DIALOGSTYLE_H

#include <QDialog>
#include "src/widget/gui.h"

class DialogStyle : public QDialog {
    Q_OBJECT

public:
    DialogStyle(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) : QDialog(parent, f)
    {
        connect(&GUI::getInstance(), &GUI::themeReload, this, &DialogStyle::reloadTheme);
    }
    virtual ~DialogStyle() {}

public slots:
    virtual void reloadTheme() {}
};

#endif //DIALOGSTYLE_H
