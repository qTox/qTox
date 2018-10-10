#include "aboutfriendform.h"
#include "ui_aboutfriendform.h"
#include "src/core/core.h"
#include "src/widget/gui.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

AboutFriendForm::AboutFriendForm(std::unique_ptr<IAboutFriend> _about, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutFriendForm)
    , about{std::move(_about)}
{
    ui->setupUi(this);
    ui->label_4->hide();
    ui->aliases->hide();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AboutFriendForm::onAcceptedClicked);
    connect(ui->autoacceptfile, &QCheckBox::clicked, this, &AboutFriendForm::onAutoAcceptClicked);
    connect(ui->autoacceptcall, SIGNAL(activated(int)), this, SLOT(onAutoAcceptCallClicked(void)));
    connect(ui->autogroupinvite, &QCheckBox::clicked, this, &AboutFriendForm::onAutoGroupInvite);
    connect(ui->selectSaveDir, &QPushButton::clicked, this, &AboutFriendForm::onSelectDirClicked);
    connect(ui->openDir, &QPushButton::clicked, this, &AboutFriendForm::onOpenDirClicked);
    connect(ui->removeHistory, &QPushButton::clicked, this, &AboutFriendForm::onRemoveHistoryClicked);
    connect(ui->resetSaveDir, &QPushButton::clicked, this, &AboutFriendForm::onResetSaveDirClicked);
    about->connectTo_autoAcceptDirChanged([=](const QString& dir) { onAutoAcceptDirChanged(dir); });

    ui->autoacceptfile->setChecked(about->getAutoAcceptEnable());

    ui->removeHistory->setEnabled(about->isHistoryExistence());

    const int index = static_cast<int>(about->getAutoAcceptCall());
    ui->autoacceptcall->setCurrentIndex(index);

    ui->autogroupinvite->setChecked(about->getAutoGroupInvite());

    ui->selectSaveDir->setText(about->getAutoAcceptDir());

    const QString name = about->getName();
    setWindowTitle(name);
    ui->userName->setText(name);
    ui->publicKey->setText(about->getPublicKey().toString());
    ui->publicKey->setCursorPosition(0); // scroll textline to left
    ui->note->setPlainText(about->getNote());
    ui->statusMessage->setText(about->getStatusMessage());
    ui->avatar->setPixmap(about->getAvatar());
}

static QString getAutoAcceptDir(const QString& dir)
{
    //: popup title
    const QString title = AboutFriendForm::tr("Choose an auto accept directory");
    return QFileDialog::getExistingDirectory(Q_NULLPTR, title, dir);
}

void AboutFriendForm::onAutoAcceptClicked()
{
    about->setAutoAcceptEnable(ui->autoacceptfile->isChecked());
}

void AboutFriendForm::onAutoAcceptDirChanged(const QString& path)
{
    if (!path.isNull()) {
        ui->selectSaveDir->setText(path);
    }
}


void AboutFriendForm::onAutoAcceptCallClicked()
{
    const int index = ui->autoacceptcall->currentIndex();
    const IFriendSettings::AutoAcceptCallFlags flag{index};
    about->setAutoAcceptCall(flag);
}

/**
 * @brief Sets the AutoGroupinvite status and saves the settings.
 */
void AboutFriendForm::onAutoGroupInvite()
{
    about->setAutoGroupInvite(ui->autogroupinvite->isChecked());
}

void AboutFriendForm::onSelectDirClicked()
{
    const QString dir = getAutoAcceptDir(about->getAutoAcceptDir());
    if (dir.isNull()) {
        return;
    }

    about->setAutoAcceptDir(dir);
}

void AboutFriendForm::onOpenDirClicked()
{
    auto dir = about->getAutoAcceptDir();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void AboutFriendForm::onResetSaveDirClicked()
{
    about->setAutoAcceptDir(QString());
}

/**
 * @brief Called when user clicks the bottom OK button, save all settings
 */
void AboutFriendForm::onAcceptedClicked()
{
    about->setNote(ui->note->toPlainText());
}

void AboutFriendForm::onRemoveHistoryClicked()
{
    const bool retYes =
        GUI::askQuestion(tr("Confirmation"),
                         tr("Are you sure to remove %1 chat history?").arg(about->getName()),
                         /* defaultAns = */ false, /* warning = */ true, /* yesno = */ true);
    if (!retYes) {
        return;
    }

    const bool result = about->clearHistory();

    if (!result) {
        GUI::showWarning(tr("History removed"),
                         tr("Failed to remove chat history with %1!").arg(about->getName()).toHtmlEscaped());
        return;
    }

    emit histroyRemoved();

    ui->removeHistory->setEnabled(false); // For know clearly to has removed the history
}

AboutFriendForm::~AboutFriendForm()
{
    delete ui;
}
