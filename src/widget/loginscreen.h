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

private:
    Ui::LoginScreen *ui;
};

#endif // LOGINSCREEN_H
