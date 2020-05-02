/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "profileform.h"
#include "ui_profileform.h"
#include "src/core/core.h"
#include "src/model/profile/iprofileinfo.h"
#include "src/persistence/profile.h"
#include "src/persistence/profilelocker.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/form/setpassworddialog.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/gui.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QImageReader>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QWindow>

static const QMap<IProfileInfo::SetAvatarResult, QString> SET_AVATAR_ERROR = {
    { IProfileInfo::SetAvatarResult::CanNotOpen,
      ProfileForm::tr("Unable to open this file.") },
    { IProfileInfo::SetAvatarResult::CanNotRead,
      ProfileForm::tr("Unable to read this image.") },
    { IProfileInfo::SetAvatarResult::TooLarge,
      ProfileForm::tr("The supplied image is too large.\nPlease use another image.") },
    { IProfileInfo::SetAvatarResult::EmptyPath,
      ProfileForm::tr("Empty path is unavaliable") },
};

static const QMap<IProfileInfo::RenameResult, QPair<QString, QString>> RENAME_ERROR = {
    { IProfileInfo::RenameResult::Error,
      { ProfileForm::tr("Failed to rename"),
        ProfileForm::tr("Couldn't rename the profile to \"%1\"") }
    } ,
    { IProfileInfo::RenameResult::ProfileAlreadyExists,
      { ProfileForm::tr("Profile already exists"),
        ProfileForm::tr("A profile named \"%1\" already exists.") }
    },
    { IProfileInfo::RenameResult::EmptyName,
      { ProfileForm::tr("Empty name"),
        ProfileForm::tr("Empty name is unavaliable") }
    },
};

static const QMap<IProfileInfo::SaveResult, QPair<QString, QString>> SAVE_ERROR = {
    { IProfileInfo::SaveResult::NoWritePermission,
      { ProfileForm::tr("Location not writable", "Title of permissions popup"),
        ProfileForm::tr("You do not have permission to write to that location. Choose "
        "another, or cancel the save dialog.", "text of permissions popup") },
    },
    { IProfileInfo::SaveResult::Error,
      { ProfileForm::tr("Failed to save file"),
        ProfileForm::tr("The file you chose could not be saved.") }
    },
    { IProfileInfo::SaveResult::EmptyPath,
      { ProfileForm::tr("Empty path"),
        ProfileForm::tr("Empty path is unavaliable.") }
    },
};

static const QPair<QString, QString> CAN_NOT_CHANGE_PASSWORD = {
    ProfileForm::tr("Couldn't change password"),
    ProfileForm::tr("Couldn't change database password, "
    "it may be corrupted or use the old password.")
};

ProfileForm::ProfileForm(IProfileInfo* profileInfo, QWidget* parent)
    : QWidget{parent}
    , qr{nullptr}
    , profileInfo{profileInfo}
{
    bodyUI = new Ui::IdentitySettings;
    bodyUI->setupUi(this);

    const uint32_t maxNameLength = tox_max_name_length();
    const QString toolTip = tr("Tox user names cannot exceed %1 characters.").arg(maxNameLength);
    bodyUI->userNameLabel->setToolTip(toolTip);
    bodyUI->userName->setMaxLength(static_cast<int>(maxNameLength));

    // tox
    toxId = new ClickableTE();
    toxId->setFont(Style::getFont(Style::Small));
    toxId->setToolTip(bodyUI->toxId->toolTip());

    QVBoxLayout* toxIdGroup = qobject_cast<QVBoxLayout*>(bodyUI->toxGroup->layout());
    delete toxIdGroup->replaceWidget(bodyUI->toxId, toxId); // Original toxId is in heap, delete it
    bodyUI->toxId->hide();

    bodyUI->qrLabel->setWordWrap(true);

    profilePicture = new MaskablePixmapWidget(this, QSize(64, 64), ":/img/avatar_mask.svg");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.svg"));
    profilePicture->setContextMenuPolicy(Qt::CustomContextMenu);
    profilePicture->setClickable(true);
    profilePicture->setObjectName("selfAvatar");
    profilePicture->installEventFilter(this);
    profilePicture->setAccessibleName("Profile avatar");
    profilePicture->setAccessibleDescription("Set a profile avatar shown to all contacts");
    profilePicture->setStyleSheet(Style::getStylesheet("window/profile.css"));
    connect(profilePicture, &MaskablePixmapWidget::clicked, this, &ProfileForm::onAvatarClicked);
    connect(profilePicture, &MaskablePixmapWidget::customContextMenuRequested,
            this, &ProfileForm::showProfilePictureContextMenu);

    QHBoxLayout* publicGrouplayout = qobject_cast<QHBoxLayout*>(bodyUI->publicGroup->layout());
    publicGrouplayout->insertWidget(0, profilePicture);
    publicGrouplayout->insertSpacing(1, 7);

    timer.setInterval(750);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, [=]() {
        bodyUI->toxIdLabel->setText(bodyUI->toxIdLabel->text().replace(" ✔", ""));
        hasCheck = false;
    });

    connect(bodyUI->toxIdLabel, &CroppingLabel::clicked, this, &ProfileForm::copyIdClicked);
    connect(toxId, &ClickableTE::clicked, this, &ProfileForm::copyIdClicked);
    profileInfo->connectTo_idChanged(this, [=](const ToxId& id) { setToxId(id); });
    connect(bodyUI->userName, &QLineEdit::editingFinished, this, &ProfileForm::onUserNameEdited);
    connect(bodyUI->statusMessage, &QLineEdit::editingFinished,
            this, &ProfileForm::onStatusMessageEdited);
    connect(bodyUI->renameButton, &QPushButton::clicked, this, &ProfileForm::onRenameClicked);
    connect(bodyUI->exportButton, &QPushButton::clicked, this, &ProfileForm::onExportClicked);
    connect(bodyUI->deleteButton, &QPushButton::clicked, this, &ProfileForm::onDeleteClicked);
    connect(bodyUI->logoutButton, &QPushButton::clicked, this, &ProfileForm::onLogoutClicked);
    connect(bodyUI->deletePassButton, &QPushButton::clicked,
            this, &ProfileForm::onDeletePassClicked);
    connect(bodyUI->changePassButton, &QPushButton::clicked,
            this, &ProfileForm::onChangePassClicked);
    connect(bodyUI->deletePassButton, &QPushButton::clicked,
            this, &ProfileForm::setPasswordButtonsText);
    connect(bodyUI->changePassButton, &QPushButton::clicked,
            this, &ProfileForm::setPasswordButtonsText);
    connect(bodyUI->saveQr, &QPushButton::clicked, this, &ProfileForm::onSaveQrClicked);
    connect(bodyUI->copyQr, &QPushButton::clicked, this, &ProfileForm::onCopyQrClicked);

    profileInfo->connectTo_usernameChanged(
            this,
            [=](const QString& val) { bodyUI->userName->setText(val); });
    profileInfo->connectTo_statusMessageChanged(
            this,
            [=](const QString& val) { bodyUI->statusMessage->setText(val); });

    for (QComboBox* cb : findChildren<QComboBox*>()) {
        cb->installEventFilter(this);
        cb->setFocusPolicy(Qt::StrongFocus);
    }

    retranslateUi();
    Translator::registerHandler(std::bind(&ProfileForm::retranslateUi, this), this);
}

void ProfileForm::prFileLabelUpdate()
{
    const QString name = profileInfo->getProfileName();
    bodyUI->prFileLabel->setText(tr("Current profile: ") + name + ".tox");
}

ProfileForm::~ProfileForm()
{
    Translator::unregister(this);
    delete qr;
    delete bodyUI;
}

bool ProfileForm::isShown() const
{
    if (profilePicture->isVisible()) {
        window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void ProfileForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(this);
    QWidget::show();
    prFileLabelUpdate();
    bool portable = Settings::getInstance().getMakeToxPortable();
    QString defaultPath = QDir(Settings::getInstance().getPaths().getSettingsDirPath()).path().trimmed();
    QString appPath = QApplication::applicationDirPath();
    QString dirPath = portable ? appPath : defaultPath;

    QString dirPrLink =
        tr("Current profile location: %1").arg(QString("<a href=\"file://%1\">%1</a>").arg(dirPath));

    bodyUI->dirPrLink->setText(dirPrLink);
    bodyUI->dirPrLink->setOpenExternalLinks(true);
    bodyUI->dirPrLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
    bodyUI->dirPrLink->setMaximumSize(bodyUI->dirPrLink->sizeHint());
    bodyUI->userName->setFocus();
    bodyUI->userName->selectAll();
}

bool ProfileForm::eventFilter(QObject* object, QEvent* event)
{
    if (object == static_cast<QObject*>(profilePicture) && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton)
            return true;
    }
    return false;
}

void ProfileForm::showProfilePictureContextMenu(const QPoint& point)
{
    const QPoint pos = profilePicture->mapToGlobal(point);

    QMenu contextMenu;
    const QIcon icon = style()->standardIcon(QStyle::SP_DialogCancelButton);
    const QAction* removeAction = contextMenu.addAction(icon, tr("Remove"));
    const QAction* selectedItem = contextMenu.exec(pos);

    if (selectedItem == removeAction) {
        profileInfo->removeAvatar();
    }
}

void ProfileForm::copyIdClicked()
{
    profileInfo->copyId();
    if (!hasCheck) {
        bodyUI->toxIdLabel->setText(bodyUI->toxIdLabel->text() + " ✔");
        hasCheck = true;
    }

    timer.start();
}

void ProfileForm::onUserNameEdited()
{
    profileInfo->setUsername(bodyUI->userName->text());
}

void ProfileForm::onStatusMessageEdited()
{
    profileInfo->setStatusMessage(bodyUI->statusMessage->text());
}

void ProfileForm::onSelfAvatarLoaded(const QPixmap& pic)
{
    profilePicture->setPixmap(pic);
}

void ProfileForm::setToxId(const ToxId& id)
{
    QString idString = id.toString();
    static const QString ToxIdColor = QStringLiteral("%1"
                                                     "<span style='color:blue'>%2</span>"
                                                     "<span style='color:gray'>%3</span>");
    toxId->setText(ToxIdColor
      .arg(idString.mid(0, 64))
      .arg(idString.mid(64, 8))
      .arg(idString.mid(72, 4)));

    delete qr;
    qr = new QRWidget();
    qr->setQRData("tox:" + idString);
    bodyUI->qrCode->setPixmap(QPixmap::fromImage(qr->getImage()->scaledToWidth(150)));
}

QString ProfileForm::getSupportedImageFilter()
{
    QString res;
    for (auto type : QImageReader::supportedImageFormats()) {
        res += QString("*.%1 ").arg(QString(type));
    }

    return tr("Images (%1)", "filetype filter").arg(res.left(res.size() - 1));
}

void ProfileForm::onAvatarClicked()
{
    const QString filter = getSupportedImageFilter();
    const QString path = QFileDialog::getOpenFileName(Q_NULLPTR, tr("Choose a profile picture"),
                                                QDir::homePath(), filter, nullptr);

    if (path.isEmpty()) {
        return;
    }
    const IProfileInfo::SetAvatarResult result = profileInfo->setAvatar(path);
    if (result == IProfileInfo::SetAvatarResult::OK) {
        return;
    }

    GUI::showError(tr("Error"), SET_AVATAR_ERROR[result]);
}

void ProfileForm::onRenameClicked()
{
    const QString cur = profileInfo->getProfileName();
    const QString title = tr("Rename \"%1\"", "renaming a profile").arg(cur);
    const QString name = QInputDialog::getText(this, title, title + ":");
    if (name.isEmpty()) {
        return;
    }

    const IProfileInfo::RenameResult result = profileInfo->renameProfile(name);
    if (result == IProfileInfo::RenameResult::OK) {
        return;
    }

    const QPair<QString, QString> error = RENAME_ERROR[result];
    GUI::showError(error.first, error.second.arg(name));
    prFileLabelUpdate();
}

void ProfileForm::onExportClicked()
{
    const QString current = profileInfo->getProfileName() + Core::TOX_EXT;
    //:save dialog title
    const QString path = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Export profile"), current,
                                                      //: save dialog filter
                                                      tr("Tox save file (*.tox)"));
    if (path.isEmpty()) {
        return;
    }

    const IProfileInfo::SaveResult result = profileInfo->exportProfile(path);
    if (result == IProfileInfo::SaveResult::OK) {
        return;
    }

    const QPair<QString, QString> error = SAVE_ERROR[result];
    GUI::showWarning(error.first, error.second);
}

void ProfileForm::onDeleteClicked()
{
    const QString title = tr("Delete profile", "deletion confirmation title");
    const QString question = tr("Are you sure you want to delete this profile?",
                            "deletion confirmation text");
    if (!GUI::askQuestion(title, question)) {
        return;
    }

    const QStringList manualDeleteFiles = profileInfo->removeProfile();
    if (manualDeleteFiles.empty()) {
        return;
    }

    //: deletion failed text part 1
    QString message = tr("The following files could not be deleted:") + "\n\n";
    for (const QString& file : manualDeleteFiles) {
        message += file + "\n";
    }

    //: deletion failed text part 2
    message += "\n" + tr("Please manually remove them.");

    GUI::showError(tr("Files could not be deleted!", "deletion failed title"), message);
}

void ProfileForm::onLogoutClicked()
{
    profileInfo->logout();
}

void ProfileForm::setPasswordButtonsText()
{
    if (profileInfo->isEncrypted()) {
        bodyUI->changePassButton->setText(tr("Change password", "button text"));
        bodyUI->deletePassButton->setVisible(true);
    } else {
        bodyUI->changePassButton->setText(tr("Set profile password", "button text"));
        bodyUI->deletePassButton->setVisible(false);
    }
}

void ProfileForm::onCopyQrClicked()
{
    profileInfo->copyQr(*qr->getImage());
}

void ProfileForm::onSaveQrClicked()
{
    const QString current = profileInfo->getProfileName() + ".png";
    const QString path = QFileDialog::getSaveFileName(
                Q_NULLPTR, tr("Save", "save qr image"), current,
                tr("Save QrCode (*.png)", "save dialog filter"));
    if (path.isEmpty()) {
        return;
    }

    const IProfileInfo::SaveResult result = profileInfo->saveQr(*qr->getImage(), path);
    if (result == IProfileInfo::SaveResult::OK) {
        return;
    }

    const QPair<QString, QString> error = SAVE_ERROR[result];
    GUI::showWarning(error.first, error.second);
}

void ProfileForm::onDeletePassClicked()
{
    if (!profileInfo->isEncrypted()) {
        GUI::showInfo(tr("Nothing to remove"), tr("Your profile does not have a password!"));
        return;
    }

    const QString title = tr("Remove password", "deletion confirmation title");
    //: deletion confirmation text
    const QString body = tr("Are you sure you want to remove your password?");
    if (!GUI::askQuestion(title, body)) {
        return;
    }

    if (!profileInfo->deletePassword()) {
        GUI::showInfo(CAN_NOT_CHANGE_PASSWORD.first, CAN_NOT_CHANGE_PASSWORD.second);
    }
}

void ProfileForm::onChangePassClicked()
{
    const QString title = tr("Please enter a new password.");
    SetPasswordDialog* dialog = new SetPasswordDialog(title, QString{}, nullptr);
    if (dialog->exec() == QDialog::Rejected) {
        return;
    }

    QString newPass = dialog->getPassword();
    if (!profileInfo->setPassword(newPass)) {
        GUI::showInfo(CAN_NOT_CHANGE_PASSWORD.first, CAN_NOT_CHANGE_PASSWORD.second);
    }
}

void ProfileForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
    setPasswordButtonsText();
    // We have to add the toxId tooltip here and not in the .ui or Qt won't know how to translate it
    // dynamically
    toxId->setToolTip(tr("This ID allows other Tox users to add and contact you.\n"
                         "Share it with your friends to begin chatting.\n\n"
                         "This ID includes the NoSpam code (in blue), and the checksum (in gray)."));
}
