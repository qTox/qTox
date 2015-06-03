#include "loginscreen.h"
#include "ui_loginscreen.h"

LoginScreen::LoginScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginScreen)
{
    ui->setupUi(this);

    connect(ui->newProfilePgbtn, &QPushButton::clicked, this, &LoginScreen::onNewProfilePageClicked);
    connect(ui->loginPgbtn, &QPushButton::clicked, this, &LoginScreen::onLoginPageClicked);
    connect(ui->createAccountButton, &QPushButton::clicked, this, &LoginScreen::onCreateNewProfile);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginScreen::onLogin);
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
