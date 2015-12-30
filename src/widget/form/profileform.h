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
class ContentLayout;

namespace Ui {
class IdentitySettings;
}

class ClickableTE : public QLineEdit
{
    Q_OBJECT
public:

signals:
    void clicked();

protected:
    virtual void mouseReleaseEvent(QMouseEvent*) final override {emit clicked();}
};

class ProfileForm : public QWidget
{
    Q_OBJECT
public:
    ProfileForm(QWidget *parent = nullptr);
    ~ProfileForm();
    virtual void show() final{}
    void show(ContentLayout* contentLayout);
    bool isShown() const;

signals:
    void userNameChanged(QString);
    void statusMessageChanged(QString);

public slots:
    void onSelfAvatarLoaded(const QPixmap &pic);
    void onLogoutClicked();

private slots:
    void setToxId(const QString& id);
    void copyIdClicked();
    void onUserNameEdited();
    void onStatusMessageEdited();
    void onRenameClicked();
    void onExportClicked();
    void onDeleteClicked();
    void onCopyQrClicked();
    void onSaveQrClicked();
    void onDeletePassClicked();
    void onChangePassClicked();
    void onAvatarClicked();
    void showProfilePictureContextMenu(const QPoint &point);
    void onRegisterButtonClicked();

private:
    void retranslateUi();
    void prFileLabelUpdate();

private:
    bool eventFilter(QObject *object, QEvent *event);
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
