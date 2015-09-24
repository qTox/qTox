#ifndef DELETEFRIENDDIALOG_H
#define DELETEFRIENDDIALOG_H


#include <QDialog>
#include "ui_removefrienddialog.h"
#include "src/friend.h"


class RemoveFriendDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RemoveFriendDialog(QWidget *parent, const Friend* f);

    inline bool removeHistory()
    {
        return ui.removeHistory->isChecked();
    }

    inline bool accepted()
    {
        return _accepted;
    }

public slots:
    void onAccepted();

protected:
    Ui_RemoveFriendDialog ui;
    bool _accepted = false;
};

#endif // DELETEFRIENDDIALOG_H
