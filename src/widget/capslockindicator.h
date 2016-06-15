#ifndef CAPSLOCKINDICATOR_H
#define CAPSLOCKINDICATOR_H

#include <QAction>
#include <QLineEdit>

class CapsLockIndicator : QAction
{
public:
    CapsLockIndicator(QLineEdit *widget);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void updateIndicator();

private:
    QLineEdit *parent;
};
#endif // CAPSLOCKINDICATOR_H
