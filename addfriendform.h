#ifndef ADDFRIENDFORM_H
#define ADDFRIENDFORM_H

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

#include "ui_widget.h"

class AddFriendForm : public QObject
{
    Q_OBJECT
public:
    AddFriendForm();

    void show(Ui::Widget& ui);

signals:
    void friendRequested(const QString& friendAddress, const QString& message);

private slots:
     void onSendTriggered();

private:
    QLabel headLabel, toxIdLabel, messageLabel;
    QPushButton sendButton;
    QLineEdit toxId;
    QTextEdit message;
    QVBoxLayout layout, headLayout;
    QWidget *head, *main;
};

#endif // ADDFRIENDFORM_H
