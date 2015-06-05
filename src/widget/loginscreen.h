#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QWidget>
#include <QShortcut>

namespace Ui {
class LoginScreen;
}

class LoginScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LoginScreen(QWidget *parent = 0);
    ~LoginScreen();
    void reset(); ///< Resets the UI, clears all fields

private slots:
    void onAutoLoginToggled(int state);
    void onLoginUsernameSelected(const QString& name);
    void onPasswordEdited();
    // Buttons to change page
    void onNewProfilePageClicked();
    void onLoginPageClicked();
    // Buttons to submit form
    void onCreateNewProfile();
    void onLogin();

private:
    void retranslateUi();

private:
    Ui::LoginScreen *ui;
    QShortcut quitShortcut;
};

#endif // LOGINSCREEN_H
