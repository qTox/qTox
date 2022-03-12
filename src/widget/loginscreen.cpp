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


#include "loginscreen.h"
#include "ui_loginscreen.h"
#include "src/persistence/profile.h"
#include "src/persistence/profilelocker.h"
#include "src/persistence/settings.h"
#include "src/widget/form/setpassworddialog.h"
#include "src/widget/style.h"
#include "src/widget/tool/profileimporter.h"
#include "src/widget/translator.h"
#include "src/persistence/settings.h"
#include <QDebug>
#include <QDialog>
#include <QMessageBox>
#include <QToolButton>

LoginScreen::LoginScreen(Settings& settings_, Style& style,
    const QString& initialProfileName, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoginScreen)
    , quitShortcut{QKeySequence(Qt::CTRL + Qt::Key_Q), this}
    , settings{settings_}
{
    ui->setupUi(this);

    // permanently disables maximize button https://github.com/qTox/qTox/issues/1973
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(size());

    connect(&quitShortcut, &QShortcut::activated, this, &LoginScreen::close);
    connect(ui->newProfilePgbtn, &QPushButton::clicked, this, &LoginScreen::onNewProfilePageClicked);
    connect(ui->loginPgbtn, &QPushButton::clicked, this, &LoginScreen::onLoginPageClicked);
    connect(ui->createAccountButton, &QPushButton::clicked, this, &LoginScreen::onCreateNewProfile);
    connect(ui->newUsername, &QLineEdit::returnPressed, this, &LoginScreen::onCreateNewProfile);
    connect(ui->newPass, &QLineEdit::returnPressed, this, &LoginScreen::onCreateNewProfile);
    connect(ui->newPassConfirm, &QLineEdit::returnPressed, this, &LoginScreen::onCreateNewProfile);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginScreen::onLogin);
    connect(ui->loginUsernames, &QComboBox::currentTextChanged, this,
            &LoginScreen::onLoginUsernameSelected);
    connect(ui->loginPassword, &QLineEdit::returnPressed, this, &LoginScreen::onLogin);
    connect(ui->newPass, &QLineEdit::textChanged, this, &LoginScreen::onPasswordEdited);
    connect(ui->newPassConfirm, &QLineEdit::textChanged, this, &LoginScreen::onPasswordEdited);
    connect(ui->autoLoginCB, &QCheckBox::stateChanged, this, &LoginScreen::onAutoLoginCheckboxChanged);
    connect(ui->importButton, &QPushButton::clicked, this, &LoginScreen::onImportProfile);

    reset(initialProfileName);
    setStyleSheet(style.getStylesheet("loginScreen/loginScreen.css", settings));

    retranslateUi();
    Translator::registerHandler(std::bind(&LoginScreen::retranslateUi, this), this);
}

LoginScreen::~LoginScreen()
{
    Translator::unregister(this);
    delete ui;
}

/**
 * @brief Resets the UI, clears all fields.
 */
void LoginScreen::reset(const QString& initialProfileName)
{
    ui->newUsername->clear();
    ui->newPass->clear();
    ui->newPassConfirm->clear();
    ui->loginPassword->clear();
    ui->loginUsernames->clear();

    QStringList allProfileNames = Profile::getAllProfileNames(settings);

    if (allProfileNames.isEmpty()) {
        ui->stackedWidget->setCurrentIndex(0);
        ui->newUsername->setFocus();
    } else {
        for (const QString& profileName : allProfileNames) {
            ui->loginUsernames->addItem(profileName);
        }

        ui->loginUsernames->setCurrentText(initialProfileName);
        ui->stackedWidget->setCurrentIndex(1);
        ui->loginPassword->setFocus();
    }
}

void LoginScreen::onProfileLoaded()
{
    done(QDialog::Accepted);
}

void LoginScreen::onProfileLoadFailed()
{
    QMessageBox::critical(this, tr("Couldn't load this profile"), tr("Wrong password."));
    ui->loginPassword->setFocus();
    ui->loginPassword->selectAll();
}

void LoginScreen::onAutoLoginChanged(bool state)
{
    ui->autoLoginCB->setChecked(state);
}

bool LoginScreen::event(QEvent* event)
{
    switch (event->type()) {
#ifdef Q_OS_MAC
    case QEvent::WindowActivate:
    case QEvent::WindowStateChange:
        emit windowStateChanged(windowState());
        break;
#endif
    default:
        break;
    }

    return QWidget::event(event);
}

void LoginScreen::onNewProfilePageClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void LoginScreen::onLoginPageClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void LoginScreen::onCreateNewProfile()
{
    QString name = ui->newUsername->text();
    QString pass = ui->newPass->text();

    if (name.isEmpty()) {
        QMessageBox::critical(this, tr("Couldn't create a new profile"),
                              tr("The username must not be empty."));
        return;
    }

    if (pass.size() != 0 && pass.size() < 6) {
        QMessageBox::critical(this, tr("Couldn't create a new profile"),
                              tr("The password must be at least 6 characters long."));
        return;
    }

    if (ui->newPassConfirm->text() != pass) {
        QMessageBox::critical(this, tr("Couldn't create a new profile"),
                              tr("The passwords you've entered are different.\nPlease make sure to "
                                 "enter the same password twice."));
        return;
    }

    if (Profile::exists(name, settings.getPaths())) {
        QMessageBox::critical(this, tr("Couldn't create a new profile"),
                              tr("A profile with this name already exists."));
        return;
    }

    emit createNewProfile(name, pass);
}

void LoginScreen::onLoginUsernameSelected(const QString& name)
{
    if (name.isEmpty())
        return;

    ui->loginPassword->clear();
    if (Profile::isEncrypted(name, settings.getPaths())) {
        ui->loginPasswordLabel->show();
        ui->loginPassword->show();
        // there is no way to do autologin if profile is encrypted, and
        // visible option confuses users into thinking that it is possible,
        // thus hide it
        ui->autoLoginCB->hide();
    } else {
        ui->loginPasswordLabel->hide();
        ui->loginPassword->hide();
        ui->autoLoginCB->show();
        ui->autoLoginCB->setToolTip(
            tr("Password protected profiles can't be automatically loaded."));
    }
}

void LoginScreen::onLogin()
{
    QString name = ui->loginUsernames->currentText();
    QString pass = ui->loginPassword->text();

    // name can be empty when there are no profiles
    if (name.isEmpty()) {
        QMessageBox::critical(this, tr("Couldn't load profile"),
                              tr("There is no selected profile.\n\n"
                                 "You may want to create one."));
        return;
    }

    if (!ProfileLocker::isLockable(name, settings.getPaths())) {
        QMessageBox::critical(this, tr("Couldn't load this profile"),
                              tr("This profile is already in use."));
        return;
    }

    emit loadProfile(name, pass);
}

void LoginScreen::onPasswordEdited()
{
    ui->passStrengthMeter->setValue(SetPasswordDialog::getPasswordStrength(ui->newPass->text()));
}

void LoginScreen::onAutoLoginCheckboxChanged(int state)
{
    auto cstate = static_cast<Qt::CheckState>(state);
    emit autoLoginChanged(cstate == Qt::CheckState::Checked);
}

void LoginScreen::retranslateUi()
{
    ui->retranslateUi(this);
}

void LoginScreen::onImportProfile()
{
    ProfileImporter pi(settings, this);
    if (pi.importProfile()) {
        reset();
    }
}
