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

#ifndef IDENTITYFORM_H
#define IDENTITYFORM_H

#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QTimer>
#include <QVBoxLayout>

#include "src/core/core.h"
#include "src/widget/qrwidget.h"

class MaskablePixmapWidget;

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
    virtual void mouseReleaseEvent(QMouseEvent*) final override
    {
        emit clicked();
    }
};

class ProfileForm : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileForm(QWidget *parent = nullptr);
    ~ProfileForm();

    QWidget* getHeadWidget();
    bool isShown() const;

signals:
    void userNameChanged(QString);
    void statusMessageChanged(QString);

public slots:
    void onSelfAvatarLoaded(const QPixmap &pic);
    void onAvatarClicked();
    void setToxId(const ToxId& id);
    void copyIdClicked();
    void onRenameClicked();
    void onExportClicked();
    void onDeleteClicked();
    void onCopyQrClicked();
    void onSaveQrClicked();
    void setPasswordButtonsText();
    void onDeletePassClicked();
    void onChangePassClicked();
    void onLogoutClicked();
    void showProfilePictureContextMenu(const QPoint &point);
    void onRegisterButtonClicked();

private slots:
    // auto-connections
    void on_userName_editingFinished();
    void on_statusMessage_editingFinished();

private:
    void showExistingToxme();
    void retranslateUi();
    void prFileLabelUpdate();

private:
    Ui::IdentitySettings* bodyUI;

    bool eventFilter(QObject *object, QEvent *event);
    void refreshProfiles();
    MaskablePixmapWidget* profilePicture;

    QTimer timer;
    bool hasCheck = false;
    QRWidget *qr;
    ClickableTE* toxId;
    void showRegisterToxme();

private:
    // indepentent head widget
    QPointer<QWidget> head;
};

#endif
