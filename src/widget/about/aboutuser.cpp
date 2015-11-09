#include "aboutuser.h"
#include "ui_aboutuser.h"
#include "src/persistence/settings.h"

AboutUser::AboutUser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutUser)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AboutUser::onAcceptedClicked);
}

void AboutUser::setFriend(Friend *f)
{
    this->setWindowTitle(f->getDisplayedName());
    ui->userName->setText(f->getDisplayedName());
    ui->publicKey->setText(QString(f->getToxId().toString()));
    ui->note->setPlainText(Settings::getInstance().getContactNote(f->getToxId()));
}

void AboutUser::setAvatar(QPixmap pixmap)
{
    ui->avatar->setPixmap(pixmap);
}

void AboutUser::setStatusMessage(QString statusMessage)
{
    ui->statusMessage->setText(statusMessage);
}

void AboutUser::setToxId(ToxId &id)
{
    this->toxId = id;
}

void AboutUser::onAcceptedClicked()
{
    Settings::getInstance().setContactNote(ui->publicKey->text(), ui->note->toPlainText());
    Settings::getInstance().saveGlobal();
}

AboutUser::~AboutUser()
{
    delete ui;
}
