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

#pragma once

#include "src/widget/qrwidget.h"
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QVBoxLayout>

class ContentLayout;
class CroppingLabel;
class IProfileInfo;
class MaskablePixmapWidget;

namespace Ui {
class IdentitySettings;
}
class ToxId;
class ClickableTE : public QLabel
{
    Q_OBJECT
public:
signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent*) final
    {
        emit clicked();
    }
};

class ProfileForm : public QWidget
{
    Q_OBJECT
public:
    ProfileForm(IProfileInfo* profileInfo, QWidget* parent = nullptr);
    ~ProfileForm();
    void show(ContentLayout* contentLayout);
    bool isShown() const;

public slots:
    void onSelfAvatarLoaded(const QPixmap& pic);
    void onLogoutClicked();

private slots:
    void setPasswordButtonsText();
    void setToxId(const ToxId& id);
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
    void showProfilePictureContextMenu(const QPoint& point);

private:
    void retranslateUi();
    void prFileLabelUpdate();
    bool eventFilter(QObject* object, QEvent* event);
    void refreshProfiles();
    static QString getSupportedImageFilter();

private:
    Ui::IdentitySettings* bodyUI;
    MaskablePixmapWidget* profilePicture;
    QTimer timer;
    bool hasCheck = false;
    QRWidget* qr;
    ClickableTE* toxId;
    IProfileInfo* profileInfo;
};
