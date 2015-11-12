#include "aboutuser.h"
#include "ui_aboutuser.h"
#include "src/persistence/settings.h"

#include <QDir>
#include <QFileDialog>

AboutUser::AboutUser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutUser)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AboutUser::onAcceptedClicked);
    connect(ui->autoaccept, &QCheckBox::clicked, this, &AboutUser::onAutoAcceptClicked);
}

void AboutUser::setFriend(Friend *f)
{
    this->setWindowTitle(f->getDisplayedName());
    ui->userName->setText(f->getDisplayedName());
    ui->publicKey->setText(QString(f->getToxId().toString()));
    ui->note->setPlainText(Settings::getInstance().getContactNote(f->getToxId()));
    QPixmap avatar = Settings::getInstance().getSavedAvatar(f->getToxId().toString());
    ui->statusMessage->setText(f->getStatusMessage());
    if(!avatar.isNull()) {
        ui->avatar->setPixmap(avatar);
    } else {
        ui->avatar->setPixmap(QPixmap(":/img/contact_dark.svg"));
    }

}

void AboutUser::setToxId(ToxId &id)
{
    this->toxId = id;
    QString dir = Settings::getInstance().getAutoAcceptDir(this->toxId);
    ui->autoaccept->setChecked(!dir.isEmpty());
}

void AboutUser::onAcceptedClicked()
{
    Settings::getInstance().setContactNote(ui->publicKey->text(), ui->note->toPlainText());
    Settings::getInstance().saveGlobal();
}

void AboutUser::onAutoAcceptClicked()
{
    QString dir;
    if (!ui->autoaccept->isChecked())
    {
        dir = QDir::homePath();
        ui->autoaccept->setChecked(false);
        Settings::getInstance().setAutoAcceptDir(this->toxId, "");
    }
    else if (ui->autoaccept->isChecked())
    {
        dir = QFileDialog::getExistingDirectory(this, tr("Choose an auto accept directory",
                                                         "popup title"), dir);
        ui->autoaccept->setChecked(true);
        Settings::getInstance().setAutoAcceptDir(this->toxId, dir);
    }
    Settings::getInstance().saveGlobal();
}

AboutUser::~AboutUser()
{
    delete ui;
}
