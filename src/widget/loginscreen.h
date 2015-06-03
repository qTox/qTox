#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QWidget>

namespace Ui {
class LoginScreen;
}

class LoginScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LoginScreen(QWidget *parent = 0);
    ~LoginScreen();

private slots:
    // Buttons to change page
    void onNewProfilePageClicked();
    void onLoginPageClicked();
    // Buttons to submit form
    void onCreateNewProfile();
    void onLogin();

private:
    Ui::LoginScreen *ui;
};

#endif // LOGINSCREEN_H
