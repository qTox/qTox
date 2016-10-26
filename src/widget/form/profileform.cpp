/*
    Copyright © 2014-2016 by The qTox Project

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

#include "src/core/core.h"
#include "src/nexus.h"
#include "ui_profileform.h"
#include "profileform.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/form/setpassworddialog.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/widget.h"
#include "src/widget/gui.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include "src/persistence/profilelocker.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/net/toxme.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QWindow>

ProfileForm::ProfileForm(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , bodyUI(new Ui::IdentitySettings)
    , qr(nullptr)
{
    const Settings& s = Settings::getInstance();
    const Core* core = Core::getInstance();

    bodyUI->setupUi(this);

    // init profile section
    profilePicture = new MaskablePixmapWidget(this, QSize(64, 64), ":/img/avatar_mask.svg");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.svg"));
    profilePicture->setContextMenuPolicy(Qt::CustomContextMenu);
    profilePicture->setClickable(true);
    profilePicture->installEventFilter(this);
    connect(profilePicture, SIGNAL(clicked()), this, SLOT(onAvatarClicked()));
    connect(profilePicture, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showProfilePictureContextMenu(const QPoint&)));
    connect(core, &Core::selfAvatarChanged,
            this, &ProfileForm::onSelfAvatarLoaded);

    QHBoxLayout *publicGrouplayout = qobject_cast<QHBoxLayout*>(bodyUI->publicGroup->layout());
    publicGrouplayout->insertWidget(0, profilePicture);
    publicGrouplayout->insertSpacing(1, 7);

    bodyUI->userName->setText(core->getUsername());
    bodyUI->userName->setFocus();
    bodyUI->userName->selectAll();
    connect(core, &Core::usernameSet, this, [=](const QString& val)
    {
        bodyUI->userName->setText(val);
    });

    bodyUI->statusMessage->setText(core->getStatusMessage());
    connect(core, &Core::statusMessageSet, this, [=](const QString& val)
    {
        bodyUI->statusMessage->setText(val);
    });

    // init profile's tox section
    toxId = new ClickableTE;
    toxId->setReadOnly(true);
    toxId->setFrame(false);
    toxId->setFont(Style::getFont(Style::Small));
    toxId->setToolTip(tr("This is the Tox-ID."));

    // TODO: declare "toxId" QLineEdit a placeholder widget in the ui file
    QLayout* toxIdLayout = bodyUI->toxGroup->layout();
    delete toxIdLayout->replaceWidget(bodyUI->toxId, toxId);
    delete bodyUI->toxId;
    bodyUI->toxId = nullptr;

    setToxId(core->getSelfId());
    connect(core, &Core::idSet, this, &ProfileForm::setToxId);
    connect(bodyUI->toxIdLabel, &CroppingLabel::clicked,
            this, &ProfileForm::copyIdClicked);
    connect(toxId, &ClickableTE::clicked,
            this, &ProfileForm::copyIdClicked);

    bodyUI->qrLabel->setWordWrap(true);

    // init toxme section
    bodyUI->toxmeServersList->addItem("toxme.io");
    QString toxmeInfo = s.getToxmeInfo();
    if (toxmeInfo.isEmpty())
        // User not registered
        showRegisterToxme();
    else
        showExistingToxme();

    QRegExp re("[^@ ]+");
    QRegExpValidator* validator = new QRegExpValidator(re, this);
    bodyUI->toxmeUsername->setValidator(validator);

    // init profile's location section
    prFileLabelUpdate();
    QString profileDir = s.getMakeToxPortable()
                         ? QApplication::applicationDirPath()
                         : QDir::cleanPath(s.getSettingsDirPath());
    QString profileLink =
            QStringLiteral("<p><a href=\"file:///") + profileDir +
            QStringLiteral("\"><span style=\"text-decoration: NONE;"
                           " color:#000000;\">") +
            tr("Current profile location: %1").arg(profileDir) +
            QStringLiteral("</span></a></p>");
    bodyUI->dirPrLink->setText(profileLink);
    bodyUI->dirPrLink->setOpenExternalLinks(true);
    bodyUI->dirPrLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse |
                                               Qt::TextSelectableByMouse);
    bodyUI->dirPrLink->setMaximumSize(bodyUI->dirPrLink->sizeHint());

    timer.setInterval(750);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, [=]() {bodyUI->toxIdLabel->setText(bodyUI->toxIdLabel->text().replace(" ✔", "")); hasCheck = false;});

    connect(bodyUI->renameButton, &QPushButton::clicked, this, &ProfileForm::onRenameClicked);
    connect(bodyUI->exportButton, &QPushButton::clicked, this, &ProfileForm::onExportClicked);
    connect(bodyUI->deleteButton, &QPushButton::clicked, this, &ProfileForm::onDeleteClicked);
    connect(bodyUI->logoutButton, &QPushButton::clicked, this, &ProfileForm::onLogoutClicked);
    connect(bodyUI->deletePassButton, &QPushButton::clicked, this, &ProfileForm::onDeletePassClicked);
    connect(bodyUI->changePassButton, &QPushButton::clicked, this, &ProfileForm::onChangePassClicked);
    connect(bodyUI->deletePassButton, &QPushButton::clicked, this, &ProfileForm::setPasswordButtonsText);
    connect(bodyUI->changePassButton, &QPushButton::clicked, this, &ProfileForm::setPasswordButtonsText);
    connect(bodyUI->saveQr, &QPushButton::clicked, this, &ProfileForm::onSaveQrClicked);
    connect(bodyUI->copyQr, &QPushButton::clicked, this, &ProfileForm::onCopyQrClicked);
    connect(bodyUI->toxmeRegisterButton, &QPushButton::clicked, this, &ProfileForm::onRegisterButtonClicked);
    connect(bodyUI->toxmeUpdateButton, &QPushButton::clicked, this, &ProfileForm::onRegisterButtonClicked);

    for (QComboBox* cb : findChildren<QComboBox*>())
    {
            cb->installEventFilter(this);
            cb->setFocusPolicy(Qt::StrongFocus);
    }

    retranslateUi();
    Translator::registerHandler(std::bind(&ProfileForm::retranslateUi, this), this);
}

void ProfileForm::prFileLabelUpdate()
{
    Nexus& nexus = Nexus::getInstance();
    bodyUI->prFileLabel->setText(tr("Current profile: %1")
                                 .arg(nexus.getProfile()->getName()) +
                                 QStringLiteral(".tox"));
}

ProfileForm::~ProfileForm()
{
    Translator::unregister(this);
    delete qr;
    delete bodyUI;
}

/**
 * @brief Returns an independent head widget.
 * @return the head widget; it will be created, if not exists
 * @note The head widget is not owned by the ProfileForm and needs to be
 * correctly free'd by the caller.
 */
QWidget* ProfileForm::getHeadWidget()
{
    if (!head)
    {
        head = new QWidget;

        QHBoxLayout* headLayout = new QHBoxLayout(head);

        QLabel* imgLabel = new QLabel;
        imgLabel->setPixmap(QPixmap(":/img/settings/identity.png")
                            .scaledToHeight(40, Qt::SmoothTransformation));
        headLayout->addWidget(imgLabel);

        QLabel* nameLabel = new QLabel(tr("User Profile"));
        QFont nameFont;
        nameFont.setBold(true);
        nameLabel->setFont(nameFont);
        headLayout->addWidget(nameLabel);
        headLayout->addStretch(1);
    }

    return head;
}

bool ProfileForm::eventFilter(QObject *object, QEvent *event)
{
    if (object == static_cast<QObject*>(profilePicture) && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton)
            return true;
    }
    return false;
}

void ProfileForm::showProfilePictureContextMenu(const QPoint &point)
{
    QPoint pos = profilePicture->mapToGlobal(point);

    QMenu contextMenu;
    QAction *removeAction = contextMenu.addAction(style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Remove"));
    QAction *selectedItem = contextMenu.exec(pos);

    if (selectedItem == removeAction)
        Nexus::getProfile()->removeAvatar();
}

void ProfileForm::copyIdClicked()
{
    toxId->selectAll();
    QString txt = toxId->text();
    txt.replace('\n',"");
    QApplication::clipboard()->setText(txt, QClipboard::Clipboard);
    if (QApplication::clipboard()->supportsSelection())
        QApplication::clipboard()->setText(txt, QClipboard::Selection);
    toxId->setCursorPosition(0);

    if (!hasCheck)
    {
        bodyUI->toxIdLabel->setText(bodyUI->toxIdLabel->text() + " ✔");
        hasCheck = true;
    }
    timer.start();
}

void ProfileForm::on_userName_editingFinished()
{
    Core::getInstance()->setUsername(bodyUI->userName->text());
}

void ProfileForm::on_statusMessage_editingFinished()
{
    Core::getInstance()->setStatusMessage(bodyUI->statusMessage->text());
}

void ProfileForm::onSelfAvatarLoaded(const QPixmap& pic)
{
    profilePicture->setPixmap(pic);
}

void ProfileForm::setToxId(const ToxId& id)
{
    const QString idStr = id.toString();
    toxId->setText(idStr);
    toxId->setCursorPosition(0);

    delete qr;
    qr = new QRWidget();
    qr->setQRData("tox:" + idStr);
    bodyUI->qrCode->setPixmap(QPixmap::fromImage(qr->getImage()->scaledToWidth(150)));
}

void ProfileForm::onAvatarClicked()
{
    auto picToPng = [](QPixmap pic)
    {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        pic.save(&buffer, "PNG");
        buffer.close();
        return bytes;
    };

    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Choose a profile picture"),
                                                    QDir::homePath(),
                                                    Nexus::getSupportedImageFilter(),
                                                    0,
                                                    QFileDialog::DontUseNativeDialog);
    if (filename.isEmpty())
        return;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
    {
        GUI::showError(tr("Error"), tr("Unable to open this file."));
        return;
    }

    QPixmap pic;
    if (!pic.loadFromData(file.readAll()))
    {
        GUI::showError(tr("Error"), tr("Unable to read this image."));
        return;
    }

    // Limit the avatar size to 64kB
    // We do a first rescale to 256x256 in case the image was huge, then keep tryng from here
    QByteArray bytes{picToPng(pic)};
    if (bytes.size() > 65535)
    {
        pic = pic.scaled(256,256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        bytes = picToPng(pic);
    }
    if (bytes.size() > 65535)
        bytes = picToPng(pic.scaled(128,128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    if (bytes.size() > 65535)
        bytes = picToPng(pic.scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    if (bytes.size() > 65535)
        bytes = picToPng(pic.scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // If this happens, you're really doing it on purpose.
    if (bytes.size() > 65535)
    {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("The supplied image is too large.\nPlease use another image."));
        return;
    }

    Nexus::getCore()->setAvatar(bytes);
}

void ProfileForm::onRenameClicked()
{
    Nexus& nexus = Nexus::getInstance();
    QString cur = nexus.getProfile()->getName();
    QString title = tr("Rename \"%1\"", "renaming a profile").arg(cur);
    do
    {
        QString name = QInputDialog::getText(this, title, title+":");
        if (name.isEmpty()) break;
        name = Core::sanitize(name);

        if (Profile::exists(name))
            GUI::showError(tr("Profile already exists", "rename failure title"),
                           tr("A profile named \"%1\" already exists.", "rename confirm text").arg(name));
        else if (!nexus.getProfile()->rename(name))
            GUI::showError(tr("Failed to rename", "rename failed title"),
                             tr("Couldn't rename the profile to \"%1\"").arg(cur));
        else
        {
            prFileLabelUpdate();
            break;
        }
    } while (true);
}

void ProfileForm::onExportClicked()
{
    QString current = Nexus::getProfile()->getName() + Core::TOX_EXT;
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Export profile", "save dialog title"),
                                                QDir::home().filePath(current),
                                                tr("Tox save file (*.tox)", "save dialog filter"),
                                                0,
                                                QFileDialog::DontUseNativeDialog);
    if (!path.isEmpty())
    {
        if (!Nexus::tryRemoveFile(path))
        {
            GUI::showWarning(tr("Location not writable","Title of permissions popup"), tr("You do not have permission to write that location. Choose another, or cancel the save dialog.", "text of permissions popup"));
            return;
        }
        if (!QFile::copy(Settings::getInstance().getSettingsDirPath()+current, path))
            GUI::showWarning(tr("Failed to copy file"), tr("The file you chose could not be written to."));
    }
}

void ProfileForm::onDeleteClicked()
{
    if (GUI::askQuestion(
                tr("Really delete profile?", "deletion confirmation title"),
                tr("Are you sure you want to delete this profile?", "deletion confirmation text")))
    {
        Nexus& nexus = Nexus::getInstance();

        QVector<QString> manualDeleteFiles = nexus.getProfile()->remove();

        if (!manualDeleteFiles.empty())
        {
            QString message = tr("The following files could not be deleted:", "deletion failed text part 1") + "\n\n";

            for (auto& file : manualDeleteFiles)
            {
                message = message + file + "\n";
            }

            message = message + "\n" + tr("Please manually remove them.", "deletion failed text part 2");

            GUI::showError(tr("Files could not be deleted!", "deletion failed title"), message);
        }

        nexus.showLoginLater();
    }
}

void ProfileForm::onLogoutClicked()
{
    Nexus& nexus = Nexus::getInstance();
    Settings::getInstance().saveGlobal();
    nexus.showLoginLater();
}

void ProfileForm::setPasswordButtonsText()
{
    if (Nexus::getProfile()->isEncrypted())
    {
        bodyUI->changePassButton->setText(tr("Change password", "button text"));
        bodyUI->deletePassButton->setVisible(true);
    }
    else
    {
        bodyUI->changePassButton->setText(tr("Set profile password", "button text"));
        bodyUI->deletePassButton->setVisible(false);
    }
}

void ProfileForm::onCopyQrClicked()
{
    QApplication::clipboard()->setImage(*qr->getImage());
}

void ProfileForm::onSaveQrClicked()
{
    QString current = Nexus::getProfile()->getName() + ".png";
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save", "save qr image"),
                                                QDir::home().filePath(current),
                                                tr("Save QrCode (*.png)", "save dialog filter"),
                                                0,
                                                QFileDialog::DontUseNativeDialog);
    if (!path.isEmpty())
    {
        if (!Nexus::tryRemoveFile(path))
        {
            GUI::showWarning(tr("Location not writable","Title of permissions popup"), tr("You do not have permission to write that location. Choose another, or cancel the save dialog.", "text of permissions popup"));
            return;
        }
        if (!qr->saveImage(path))
            GUI::showWarning(tr("Failed to copy file"), tr("The file you chose could not be written to."));
    }
}

void ProfileForm::onDeletePassClicked()
{
    Profile* pro = Nexus::getProfile();
    if (!pro->isEncrypted())
    {
        GUI::showInfo(tr("Nothing to remove"), tr("Your profile does not have a password!"));
        return;
    }

    if (!GUI::askQuestion(tr("Really delete password?","deletion confirmation title"),
                      tr("Are you sure you want to delete your password?","deletion confirmation text")))
        return;

    Nexus::getProfile()->setPassword(QString());
}

void ProfileForm::onChangePassClicked()
{
    SetPasswordDialog* dialog = new SetPasswordDialog(tr("Please enter a new password."), QString(), 0);
    int r = dialog->exec();
    if (r == QDialog::Rejected)
        return;

    QString newPass = dialog->getPassword();
    Nexus::getProfile()->setPassword(newPass);
}

void ProfileForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
    setPasswordButtonsText();
    // We have to add the toxId tooltip here and not in the .ui or Qt won't know how to translate it dynamically
    toxId->setToolTip(tr("This bunch of characters tells other Tox clients how to contact you.\nShare it with your friends to communicate."));
}

void ProfileForm::showRegisterToxme()
{
    bodyUI->toxmeUsername->setText(QString());
    bodyUI->toxmeBio->setText(QString());
    bodyUI->toxmePrivacy->setChecked(false);

    bodyUI->toxmeRegisterButton->show();
    bodyUI->toxmeUpdateButton->hide();
    bodyUI->toxmePassword->hide();
    bodyUI->toxmePasswordLabel->hide();
}

void ProfileForm::showExistingToxme()
{
    QStringList info = Settings::getInstance().getToxmeInfo().split("@");
    bodyUI->toxmeUsername->setText(info[0]);
    bodyUI->toxmeServersList->addItem(info[1]);

    QString bio = Settings::getInstance().getToxmeBio();
    bodyUI->toxmeBio->setText(bio);

    bool priv = Settings::getInstance().getToxmePriv();
    bodyUI->toxmePrivacy->setChecked(priv);

    QString pass = Settings::getInstance().getToxmePass();
    bodyUI->toxmePassword->setText(pass);
    bodyUI->toxmePassword->show();
    bodyUI->toxmePasswordLabel->show();

    bodyUI->toxmeRegisterButton->hide();
    bodyUI->toxmeUpdateButton->show();
}

void ProfileForm::onRegisterButtonClicked()
{
    QString name =  bodyUI->toxmeUsername->text();
    if (name.isEmpty())
        return;

    bodyUI->toxmeRegisterButton->setEnabled(false);
    bodyUI->toxmeUpdateButton->setEnabled(false);
    bodyUI->toxmeRegisterButton->setText(tr("Register (processing)"));
    bodyUI->toxmeUpdateButton->setText(tr("Update (processing)"));

    QString id = toxId->text();
    QString bio = bodyUI->toxmeBio->text();
    QString server = bodyUI->toxmeServersList->currentText();
    bool privacy = bodyUI->toxmePrivacy->isChecked();

    Core* oldCore = Core::getInstance();

    Toxme::ExecCode code = Toxme::ExecCode::Ok;
    QString response = Toxme::createAddress(code, server, ToxId(id), name, privacy, bio);

    Core* newCore = Core::getInstance();
    // Make sure the user didn't logout (or logout and login)
    // before the request is finished, else qTox will crash.
    if (oldCore == newCore)
    {
        switch (code)
        {
        case Toxme::Updated:
            GUI::showInfo(tr("Done!"), tr("Account %1@%2 updated successfully").arg(name, server));
            Settings::getInstance().setToxme(name, server, bio, privacy);
            showExistingToxme();
            break;
        case Toxme::Ok:
            GUI::showInfo(tr("Done!"), tr("Successfully added %1@%2 to the database. Save your password").arg(name, server));
            Settings::getInstance().setToxme(name, server, bio, privacy, response);
            showExistingToxme();
            break;
        default:
            QString errorMessage = Toxme::getErrorMessage(code);
            qWarning() << errorMessage;
            QString translated = Toxme::translateErrorMessage(code);
            GUI::showWarning(tr("Toxme error"),  translated);
        }

        bodyUI->toxmeRegisterButton->setEnabled(true);
        bodyUI->toxmeUpdateButton->setEnabled(true);
        bodyUI->toxmeRegisterButton->setText(tr("Register"));
        bodyUI->toxmeUpdateButton->setText(tr("Update"));
    }
}
