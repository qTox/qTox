#include "loginscreen.h"
#include "ui_loginscreen.h"
#include "src/profile.h"
#include "src/profilelocker.h"
#include "src/nexus.h"
#include "src/misc/settings.h"
#include "src/widget/form/setpassworddialog.h"
#include <QMessageBox>
#include <QDebug>

LoginScreen::LoginScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginScreen)
{
    ui->setupUi(this);

    connect(ui->newProfilePgbtn, &QPushButton::clicked, this, &LoginScreen::onNewProfilePageClicked);
    connect(ui->loginPgbtn, &QPushButton::clicked, this, &LoginScreen::onLoginPageClicked);
    connect(ui->createAccountButton, &QPushButton::clicked, this, &LoginScreen::onCreateNewProfile);
    connect(ui->newUsername, &QLineEdit::returnPressed, this, &LoginScreen::onCreateNewProfile);
    connect(ui->newPass, &QLineEdit::returnPressed, this, &LoginScreen::onCreateNewProfile);
    connect(ui->newPassConfirm, &QLineEdit::returnPressed, this, &LoginScreen::onCreateNewProfile);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginScreen::onLogin);
    connect(ui->loginUsernames, &QComboBox::currentTextChanged, this, &LoginScreen::onLoginUsernameSelected);
    connect(ui->loginPassword, &QLineEdit::returnPressed, this, &LoginScreen::onLogin);
    connect(ui->newPass, &QLineEdit::textChanged, this, &LoginScreen::onPasswordEdited);
    connect(ui->newPassConfirm, &QLineEdit::textChanged, this, &LoginScreen::onPasswordEdited);
    connect(ui->autoLoginCB, &QCheckBox::stateChanged, this, &LoginScreen::onAutoLoginToggled);

    reset();
}

LoginScreen::~LoginScreen()
{
    delete ui;
}

void LoginScreen::reset()
{
    ui->newUsername->clear();
    ui->newPass->clear();
    ui->newPassConfirm->clear();
    ui->loginPassword->clear();
    ui->loginUsernames->clear();

    Profile::scanProfiles();
    QString lastUsed = Settings::getInstance().getCurrentProfile();
    qDebug() << "Last used is "<<lastUsed;
    QVector<QString> profiles = Profile::getProfiles();
    for (QString profile : profiles)
    {
        ui->loginUsernames->addItem(profile);
        if (profile == lastUsed)
            ui->loginUsernames->setCurrentIndex(ui->loginUsernames->count()-1);
    }

    if (profiles.isEmpty())
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->newUsername->setFocus();
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(1);
        ui->loginPassword->setFocus();
    }

    ui->autoLoginCB->setChecked(Settings::getInstance().getAutoLogin());
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

    if (name.isEmpty())
    {
        QMessageBox::critical(this, tr("Couldn't create a new profile"), tr("The username must not be empty."));
        return;
    }

    if (pass.size()!=0 && pass.size() < 6)
    {
        QMessageBox::critical(this, tr("Couldn't create a new profile"), tr("The password must be at least 6 characters long."));
        return;
    }

    if (ui->newPassConfirm->text() != pass)
    {
        QMessageBox::critical(this, tr("Couldn't create a new profile"), tr("The passwords you've entered are different.\nPlease make sure to enter same password twice."));
        return;
    }

    if (Profile::exists(name))
    {
        QMessageBox::critical(this, tr("Couldn't create a new profile"), tr("A profile with this name already exists."));
        return;
    }

    Profile* profile = Profile::createProfile(name, pass);
    if (!profile)
    {
        // Unknown error
        QMessageBox::critical(this, tr("Couldn't create a new profile"), tr("Unknown error: Couldn't create a new profile.\nIf you encountered this error, please report it."));
        return;
    }

    Nexus& nexus = Nexus::getInstance();

    nexus.setProfile(profile);
    nexus.showMainGUI();
}

void LoginScreen::onLoginUsernameSelected(const QString &name)
{
    if (name.isEmpty())
        return;

    ui->loginPassword->clear();
    if (Profile::isEncrypted(name))
    {
        ui->loginPasswordLabel->show();
        ui->loginPassword->show();
    }
    else
    {
        ui->loginPasswordLabel->hide();
        ui->loginPassword->hide();
    }
}

void LoginScreen::onLogin()
{
    QString name = ui->loginUsernames->currentText();
    QString pass = ui->loginPassword->text();

    if (!ProfileLocker::isLockable(name))
    {
        QMessageBox::critical(this, tr("Couldn't load this profile"), tr("This profile is already in use."));
        return;
    }

    Profile* profile = Profile::loadProfile(name, pass);
    if (!profile)
    {
        // Unknown error
        QMessageBox::critical(this, tr("Couldn't load this profile"), tr("Couldn't load this profile."));
        return;
    }
    if (!profile->checkPassword())
    {
        QMessageBox::critical(this, tr("Couldn't load this profile"), tr("Wrong password."));
        delete profile;
        return;
    }

    Nexus& nexus = Nexus::getInstance();

    nexus.setProfile(profile);
    nexus.showMainGUI();
}

void LoginScreen::onPasswordEdited()
{
    ui->passStrengthMeter->setValue(SetPasswordDialog::getPasswordStrength(ui->newPass->text()));
}

void LoginScreen::onAutoLoginToggled(int state)
{
    Qt::CheckState cstate = static_cast<Qt::CheckState>(state);
    if (cstate == Qt::CheckState::Unchecked)
        Settings::getInstance().setAutoLogin(false);
    else
        Settings::getInstance().setAutoLogin(true);

    Settings::getInstance().save(false);
}
