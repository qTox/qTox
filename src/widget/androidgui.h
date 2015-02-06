#ifndef ANDROIDGUI_H
#define ANDROIDGUI_H

#include <QWidget>

class QLabel;

class AndroidGUI : public QWidget
{
    Q_OBJECT
public:
    explicit AndroidGUI(QWidget *parent = 0);
    ~AndroidGUI();

private:
    QLabel* l;
};

#endif // ANDROIDGUI_H
