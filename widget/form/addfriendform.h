#ifndef ADDFRIENDFORM_H
#define ADDFRIENDFORM_H

#include "ui_widget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QDnsLookup>

class AddFriendForm : public QObject
{
    Q_OBJECT
public:
    AddFriendForm();
    ~AddFriendForm();

    void show(Ui::Widget& ui);
    bool isToxId(const QString& value) const;
    void showWarning(const QString& message) const;
    QString getMessage() const;

signals:
    void friendRequested(const QString& friendAddress, const QString& message);

private slots:
     void onSendTriggered();
     void handleDnsLookup();

private:
    QLabel headLabel, toxIdLabel, messageLabel;
    QPushButton sendButton;
    QLineEdit toxId;
    QTextEdit message;
    QVBoxLayout layout, headLayout;
    QWidget *head, *main;

    /** will be used for dns discovery if necessary */
    QDnsLookup dns;
};

#endif // ADDFRIENDFORM_H
