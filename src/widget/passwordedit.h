#ifndef PASSWORDEDIT_H
#define PASSWORDEDIT_H

#include <QAction>
#include <QLineEdit>

class PasswordEdit : public QLineEdit
{
public:
    explicit PasswordEdit(QWidget *parent);
    ~PasswordEdit();

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

private:
    class EventHandler : QObject
    {
    public:
        QVector<QAction*> actions;

        EventHandler();
        ~EventHandler();
        void updateActions();
        bool eventFilter(QObject *obj, QEvent *event);
    };

    void registerHandler();
    void unregisterHandler();

private:
    QAction* action;

    static EventHandler* eventHandler;
};
#endif // PASSWORDEDIT_H
