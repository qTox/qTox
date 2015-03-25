/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "src/core.h"
#include "src/nexus.h"
#include "ui_profileform.h"
#include "profileform.h"
#include "ui_mainwindow.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/misc/settings.h"
#include "src/widget/croppinglabel.h"
#include "src/widget/widget.h"
#include "src/widget/gui.h"
#include "src/historykeeper.h"
#include "src/misc/style.h"
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QFileDialog>
#include <QBuffer>

void ProfileForm::refreshProfiles()
{
    bodyUI->profiles->clear();
    for (QString profile : Settings::getInstance().searchProfiles())
        bodyUI->profiles->addItem(profile);
    QString current = Settings::getInstance().getCurrentProfile();
    if (current != "")
        bodyUI->profiles->setCurrentText(current);
}

ProfileForm::ProfileForm(QWidget *parent) :
    QWidget(parent)
{
    bodyUI = new Ui::IdentitySettings;
    bodyUI->setupUi(this);
    core = Core::getInstance();

    head = new QWidget(this);
    QHBoxLayout* headLayout = new QHBoxLayout();
    head->setLayout(headLayout);

    QLabel* imgLabel = new QLabel();
    headLayout->addWidget(imgLabel);

    QLabel* nameLabel = new QLabel();
    QFont bold;
    bold.setBold(true);
    nameLabel->setFont(bold);
    headLayout->addWidget(nameLabel);
    headLayout->addStretch(1);

    nameLabel->setText(tr("User Profile"));
    imgLabel->setPixmap(QPixmap(":/img/settings/identity.png").scaledToHeight(40, Qt::SmoothTransformation));

    // tox
    toxId = new ClickableTE();
    toxId->setReadOnly(true);
    toxId->setFrame(false);
    toxId->setFont(Style::getFont(Style::Small));
    toxId->setToolTip(bodyUI->toxId->toolTip());

    QVBoxLayout *toxIdGroup = qobject_cast<QVBoxLayout*>(bodyUI->toxGroup->layout());
    toxIdGroup->replaceWidget(bodyUI->toxId, toxId);
    bodyUI->toxId->hide();

    bodyUI->qrLabel->setWordWrap(true);

    profilePicture = new MaskablePixmapWidget(this, QSize(64, 64), ":/img/avatar_mask.svg");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.svg"));
    profilePicture->setClickable(true);
    connect(profilePicture, SIGNAL(clicked()), this, SLOT(onAvatarClicked()));
    QHBoxLayout *publicGrouplayout = qobject_cast<QHBoxLayout*>(bodyUI->publicGroup->layout());
    publicGrouplayout->insertWidget(0, profilePicture);
    publicGrouplayout->insertSpacing(1, 7);

    timer.setInterval(750);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, [=]() {bodyUI->toxIdLabel->setText(bodyUI->toxIdLabel->text().replace(" ✔", "")); hasCheck = false;});

    connect(bodyUI->toxIdLabel, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(toxId, SIGNAL(clicked()), this, SLOT(copyIdClicked()));
    connect(core, &Core::idSet, this, &ProfileForm::setToxId);
    connect(core, &Core::statusSet, this, &ProfileForm::onStatusSet);
    connect(bodyUI->userName, SIGNAL(editingFinished()), this, SLOT(onUserNameEdited()));
    connect(bodyUI->statusMessage, SIGNAL(editingFinished()), this, SLOT(onStatusMessageEdited()));
    connect(bodyUI->loadButton, &QPushButton::clicked, this, &ProfileForm::onLoadClicked);
    connect(bodyUI->renameButton, &QPushButton::clicked, this, &ProfileForm::onRenameClicked);
    connect(bodyUI->exportButton, &QPushButton::clicked, this, &ProfileForm::onExportClicked);
    connect(bodyUI->deleteButton, &QPushButton::clicked, this, &ProfileForm::onDeleteClicked);
    connect(bodyUI->importButton, &QPushButton::clicked, this, &ProfileForm::onImportClicked);
    connect(bodyUI->newButton, &QPushButton::clicked, this, &ProfileForm::onNewClicked);

    connect(core, &Core::avStart, this, &ProfileForm::disableSwitching);
    connect(core, &Core::avStarting, this, &ProfileForm::disableSwitching);
    connect(core, &Core::avInvite, this, &ProfileForm::disableSwitching);
    connect(core, &Core::avRinging, this, &ProfileForm::disableSwitching);
    connect(core, &Core::avCancel, this, &ProfileForm::enableSwitching);
    connect(core, &Core::avEnd, this, &ProfileForm::enableSwitching);
    connect(core, &Core::avEnding, this, &ProfileForm::enableSwitching);
    connect(core, &Core::avPeerTimeout, this, &ProfileForm::enableSwitching);
    connect(core, &Core::avRequestTimeout, this, &ProfileForm::enableSwitching);

    connect(core, &Core::usernameSet, this, [=](const QString& val) { bodyUI->userName->setText(val); });
    connect(core, &Core::statusMessageSet, this, [=](const QString& val) { bodyUI->statusMessage->setText(val); });

    for (QComboBox* cb : findChildren<QComboBox*>())
    {
            cb->installEventFilter(this);
            cb->setFocusPolicy(Qt::StrongFocus);
    }
}

ProfileForm::~ProfileForm()
{
    delete bodyUI;
    head->deleteLater();
}

void ProfileForm::show(Ui::MainWindow &ui)
{
    ui.mainHead->layout()->addWidget(head);
    ui.mainContent->layout()->addWidget(this);
    head->show();
    QWidget::show();
    bodyUI->userName->setFocus(Qt::OtherFocusReason);
    bodyUI->userName->selectAll();
}

void ProfileForm::copyIdClicked()
{
    toxId->selectAll();
    QString txt = toxId->text();
    txt.replace('\n',"");
    QApplication::clipboard()->setText(txt);
    toxId->setCursorPosition(0);

    if (!hasCheck)
    {
        bodyUI->toxIdLabel->setText(bodyUI->toxIdLabel->text() + " ✔");
        hasCheck = true;
    }
    timer.start();
}

void ProfileForm::onUserNameEdited()
{
    Core::getInstance()->setUsername(bodyUI->userName->text());
}

void ProfileForm::onStatusMessageEdited()
{
    Core::getInstance()->setStatusMessage(bodyUI->statusMessage->text());
}

void ProfileForm::onSelfAvatarLoaded(const QPixmap& pic)
{
    profilePicture->setPixmap(pic);
}

void ProfileForm::setToxId(const QString& id)
{
    toxId->setText(id);
    toxId->setCursorPosition(0);

    qr = new QRWidget();
    qr->setQRData("tox:"+id);
    bodyUI->qrCode->setPixmap(QPixmap::fromImage(qr->getImage()->scaledToWidth(150)));
}

void ProfileForm::onAvatarClicked()
{
    QString filename = QFileDialog::getOpenFileName(0,
        tr("Choose a profile picture"),
        QDir::homePath(),
        Nexus::getSupportedImageFilter());
    if (filename.isEmpty())
        return;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
    {
        GUI::showError(tr("Error"), tr("Unable to open this file"));
        return;
    }

    QPixmap pic;
    if (!pic.loadFromData(file.readAll()))
    {
        GUI::showError(tr("Error"), tr("Unable to read this image"));
        return;
    }

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pic.save(&buffer, "PNG");
    buffer.close();

    if (bytes.size() >= TOX_AVATAR_MAX_DATA_LENGTH)
    {
        pic = pic.scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        bytes.clear();
        buffer.open(QIODevice::WriteOnly);
        pic.save(&buffer, "PNG");
        buffer.close();
    }

    if (bytes.size() >= TOX_AVATAR_MAX_DATA_LENGTH)
    {
        GUI::showError(tr("Error"), tr("This image is too big"));
        return;
    }

    Nexus::getCore()->setAvatar(TOX_AVATAR_FORMAT_PNG, bytes);
}

void ProfileForm::onLoadClicked()
{
    if (bodyUI->profiles->currentText() != Settings::getInstance().getCurrentProfile())
    {
        if (Core::getInstance()->anyActiveCalls())
            GUI::showWarning(tr("Call active", "popup title"),
                tr("You can't switch profiles while a call is active!", "popup text"));
        else
            emit Widget::getInstance()->changeProfile(bodyUI->profiles->currentText());
            // I think by directly calling the function, I may have been causing thread issues
    }
}

void ProfileForm::onRenameClicked()
{
    QString cur = bodyUI->profiles->currentText();
    QString title = tr("Rename \"%1\"", "renaming a profile").arg(cur);
    do
    {
        QString name = QInputDialog::getText(this, title, title+":");
        if (name.isEmpty()) break;
        name = Core::sanitize(name);
        QDir dir(Settings::getSettingsDirPath());
        QString file = dir.filePath(name+Core::TOX_EXT);
        if (!QFile::exists(file) || GUI::askQuestion(tr("Profile already exists", "rename confirm title"),
                tr("A profile named \"%1\" already exists. Do you want to erase it?", "rename confirm text").arg(cur)))
        {
            QFile::rename(dir.filePath(cur+Core::TOX_EXT), file);
            QFile::rename(dir.filePath(cur+".ini"), dir.filePath(name+".ini"));
            bodyUI->profiles->setItemText(bodyUI->profiles->currentIndex(), name);
            HistoryKeeper::renameHistory(cur, name);
            bool resetAutorun = Settings::getInstance().getAutorun();
            Settings::getInstance().setAutorun(false);
            Settings::getInstance().setCurrentProfile(name);
            if (resetAutorun)
                Settings::getInstance().setAutorun(true);                   // fixes -p flag in autostart command line
            break;
        }
    } while (true);
}

void ProfileForm::onExportClicked()
{
    QString current = bodyUI->profiles->currentText() + Core::TOX_EXT;
    QString path = QFileDialog::getSaveFileName(0, tr("Export profile", "save dialog title"),
                    QDir::home().filePath(current),
                    tr("Tox save file (*.tox)", "save dialog filter"));
    if (!path.isEmpty())
    {
        if (!Nexus::isFilePathWritable(path))
        {
            GUI::showWarning(tr("Location not writable","Title of permissions popup"), tr("You do not have permission to write that location. Choose another, or cancel the save dialog.", "text of permissions popup"));
            return;
        }
        if (!QFile::copy(QDir(Settings::getSettingsDirPath()).filePath(current), path))
            GUI::showWarning(tr("Failed to copy file"), tr("The file you chose could not be written to."));
    }
}

void ProfileForm::onDeleteClicked()
{
    if (Settings::getInstance().getCurrentProfile() == bodyUI->profiles->currentText())
    {
        GUI::showWarning(tr("Profile currently loaded","current profile deletion warning title"), tr("This profile is currently in use. Please load a different profile before deleting this one.","current profile deletion warning text"));
    }
    else
    {
        if (GUI::askQuestion(tr("Deletion imminent!","deletion confirmation title"),
                          tr("Are you sure you want to delete this profile?","deletion confirmation text")))
        {
            QString profile = bodyUI->profiles->currentText();
            QDir dir(Settings::getSettingsDirPath());

            QFile::remove(dir.filePath(profile + Core::TOX_EXT));
            QFile::remove(dir.filePath(profile + ".ini"));
            QFile::remove(HistoryKeeper::getHistoryPath(profile, 0));
            QFile::remove(HistoryKeeper::getHistoryPath(profile, 1));

            bodyUI->profiles->removeItem(bodyUI->profiles->currentIndex());
            bodyUI->profiles->setCurrentText(Settings::getInstance().getCurrentProfile());
        }
    }
}

void ProfileForm::onImportClicked()
{
    QString path = QFileDialog::getOpenFileName(0,
                                                tr("Import profile", "import dialog title"),
                                                QDir::homePath(),
                                                tr("Tox save file (*.tox)", "import dialog filter"));
    if (path.isEmpty())
        return;

    QFileInfo info(path);
    QString profile = info.completeBaseName();

    if (info.suffix() != "tox")
    {
        GUI::showWarning(tr("Ignoring non-Tox file", "popup title"),
                         tr("Warning: you've chosen a file that is not a Tox save file; ignoring.", "popup text"));
        return;
    }

    QString profilePath = QDir(Settings::getSettingsDirPath()).filePath(profile + Core::TOX_EXT);

    if (QFileInfo(profilePath).exists() && !GUI::askQuestion(tr("Profile already exists", "import confirm title"),
            tr("A profile named \"%1\" already exists. Do you want to erase it?", "import confirm text").arg(profile)))
        return;

    QFile::copy(path, profilePath);
    bodyUI->profiles->addItem(profile);
}

void ProfileForm::onStatusSet(Status)
{
    refreshProfiles();
}

void ProfileForm::onNewClicked()
{
    emit Widget::getInstance()->changeProfile(QString());
}

void ProfileForm::disableSwitching()
{
    bodyUI->loadButton->setEnabled(false);
    bodyUI->newButton->setEnabled(false);
}

void ProfileForm::enableSwitching()
{
    if (!core->anyActiveCalls())
    {
        bodyUI->loadButton->setEnabled(true);
        bodyUI->newButton->setEnabled(true);
    }
}

void ProfileForm::showEvent(QShowEvent *event)
{
    refreshProfiles();
    QWidget::showEvent(event);
}

void ProfileForm::on_copyQr_clicked()
{
    QApplication::clipboard()->setImage(*qr->getImage());
}

void ProfileForm::on_saveQr_clicked()
{
    QString current = bodyUI->profiles->currentText() + ".png";
    QString path = QFileDialog::getSaveFileName(0, tr("Save", "save qr image"),
                   QDir::home().filePath(current),
                   tr("Save QrCode (*.png)", "save dialog filter"));
    if (!path.isEmpty())
    {
        if (!Nexus::isFilePathWritable(path))
        {
            GUI::showWarning(tr("Location not writable","Title of permissions popup"), tr("You do not have permission to write that location. Choose another, or cancel the save dialog.", "text of permissions popup"));
            return;
        }
        if (!qr->saveImage(path))
            GUI::showWarning(tr("Failed to copy file"), tr("The file you chose could not be written to."));
    }
}

bool ProfileForm::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Wheel) &&
         (qobject_cast<QComboBox*>(o) || qobject_cast<QAbstractSpinBox*>(o) ))
    {
        e->ignore();
        return true;
    }
    return QWidget::eventFilter(o, e);
}
