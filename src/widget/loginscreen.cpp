#include "loginscreen.h"
#include "ui_loginscreen.h"
#include "src/profile.h"

LoginScreen::LoginScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginScreen)
{
    ui->setupUi(this);

    connect(ui->newProfilePgbtn, &QPushButton::clicked, this, &LoginScreen::onNewProfilePageClicked);
    connect(ui->loginPgbtn, &QPushButton::clicked, this, &LoginScreen::onLoginPageClicked);
    connect(ui->createAccountButton, &QPushButton::clicked, this, &LoginScreen::onCreateNewProfile);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginScreen::onLogin);

    QVector<QString> profiles = Profile::getProfiles();
    for (QString profile : profiles)
        ui->loginUsernames->addItem(profile);

    if (profiles.isEmpty())
        ui->stackedWidget->setCurrentIndex(0);
    else
        ui->stackedWidget->setCurrentIndex(1);
}

LoginScreen::~LoginScreen()
{
    delete ui;
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

}

void LoginScreen::onLogin()
{

}
