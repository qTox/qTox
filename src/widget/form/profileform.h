/*
    Copyright © 2014-2015 by The qTox Project

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

#ifndef IDENTITYFORM_H
#define IDENTITYFORM_H

#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include "src/core/core.h"
#include "src/widget/qrwidget.h"

class CroppingLabel;
class Core;
class MaskablePixmapWidget;

namespace Ui {
class IdentitySettings;
class MainWindow;
}

class ClickableTE : public QLineEdit
{
    Q_OBJECT
public:

signals:
    void clicked();
protected:
    void mouseReleaseEvent(QMouseEvent*) {emit clicked();}
};

class ProfileForm : public QWidget
{
    Q_OBJECT
public:
    ProfileForm(QWidget *parent = nullptr);
    ~ProfileForm();
    void show(Ui::MainWindow &ui);

signals:
    void userNameChanged(QString);
    void statusMessageChanged(QString);

public slots:
    void onSelfAvatarLoaded(const QPixmap &pic);

private slots:
    void setToxId(const QString& id);
    void copyIdClicked();
    void onAvatarClicked();
    void onUserNameEdited();
    void onStatusMessageEdited();
    void onRenameClicked();
    void onExportClicked();
    void onDeleteClicked();
    void onLogoutClicked();
    void onCopyQrClicked();
    void onSaveQrClicked();
    void onDeletePassClicked();
    void onChangePassClicked();

private:
    void retranslateUi();

private:
    void refreshProfiles();
    Ui::IdentitySettings* bodyUI;
    MaskablePixmapWidget* profilePicture;
    QLabel* nameLabel;
    QWidget *head;
    Core* core;
    QTimer timer;
    bool hasCheck = false;
    QRWidget *qr;
    ClickableTE* toxId;
};

#endif
