#ifndef CAPSLOCKINDICATOR_H
#define CAPSLOCKINDICATOR_H

#include <QToolButton>

class CapsLockIndicator : QToolButton
{
public:
    CapsLockIndicator(QWidget *widget);
    void updateIndicator();

private:
    void show();
    void hide();

private:
    QString cleanInputStyle;
    QSize inputSize;
};
#endif // CAPSLOCKINDICATOR_H
