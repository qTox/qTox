#ifndef CAPSLOCKINDICATOR_H
#define CAPSLOCKINDICATOR_H

#include <QAction>
#include <QLineEdit>

class CapsLockIndicator : public QAction
{
public:
    CapsLockIndicator(QObject *parent);
    ~CapsLockIndicator();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void updateIndicator();
};
#endif // CAPSLOCKINDICATOR_H
