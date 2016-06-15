#ifndef CAPSLOCKINDICATOR_H
#define CAPSLOCKINDICATOR_H

#include <QAction>
#include <QLineEdit>

class CapsLockIndicator : public QAction
{
public:
    CapsLockIndicator(QObject *parent);
    ~CapsLockIndicator();

private:
    class EventHandler : QObject
    {
    public:
        QVector<QAction*> actions;

        EventHandler();
        ~EventHandler();
        void updateActions(const QObject* object = nullptr);
        bool eventFilter(QObject *obj, QEvent *event);
    };

private:
    static EventHandler* eventHandler;
};
#endif // CAPSLOCKINDICATOR_H
