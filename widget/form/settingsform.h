#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QObject>
#include <QSpacerItem>
#include "ui_widget.h"

class SettingsForm : public QObject
{
    Q_OBJECT
public:
    SettingsForm();
    ~SettingsForm();

    void show(Ui::Widget& ui);

public slots:
    void setFriendAddress(const QString& friendAddress);

private:
    QLabel headLabel, nameLabel, statusTextLabel, idLabel, id;
    QVBoxLayout layout, headLayout;
    QWidget *main, *head;

public:
    QLineEdit name, statusText;
};

#endif // SETTINGSFORM_H
