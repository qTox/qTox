/*
    Copyright © 2015-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/


#pragma once

#include <QDialog>
#include <QShortcut>
#include <QToolButton>

class Profile;

namespace Ui {
class LoginScreen;
}

class LoginScreen : public QDialog
{
    Q_OBJECT

public:
    LoginScreen(const QString& initialProfileName = QString(), QWidget* parent = nullptr);
    ~LoginScreen();
    bool event(QEvent* event) final;

signals:

    void windowStateChanged(Qt::WindowStates states);
    void autoLoginChanged(bool state);
    void createNewProfile(QString name, const QString& pass);
    void loadProfile(QString name, const QString& pass);

public slots:
    void onProfileLoaded();
    void onProfileLoadFailed();
    void onAutoLoginChanged(bool state);

private slots:
    void onAutoLoginCheckboxChanged(int state);
    void onLoginUsernameSelected(const QString& name);
    void onPasswordEdited();
    // Buttons to change page
    void onNewProfilePageClicked();
    void onLoginPageClicked();
    // Buttons to submit form
    void onCreateNewProfile();
    void onLogin();
    void onImportProfile();

private:
    void reset(const QString& initialProfileName = QString());
    void retranslateUi();
    void showCapsIndicator();
    void hideCapsIndicator();
    void checkCapsLock();

private:
    Ui::LoginScreen* ui;
    QShortcut quitShortcut;
};
