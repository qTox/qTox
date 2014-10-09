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

#ifndef IDENTITYFORM_H
#define IDENTITYFORM_H

#include "genericsettings.h"
#include <QGroupBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>

class CroppingLabel;

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
    void mouseReleaseEvent(QMouseEvent*) {emit clicked();}    
};

class IdentityForm : public GenericForm
{
    Q_OBJECT
public:
    IdentityForm();
    ~IdentityForm();

    void setUserName(const QString &name);
    void setStatusMessage(const QString &msg);

    virtual void updateContent();

signals:
    void userNameChanged(QString);
    void statusMessageChanged(QString);

private slots:
    void copyIdClicked();
    void onUserNameEdited();
    void onStatusMessageEdited();
    void onLoadClicked();
    void onRenameClicked();
    void onExportClicked();
    void onDeleteClicked();
    void onImportClicked();

private:
    Ui::IdentitySettings* bodyUI;

    ClickableTE* toxId;
};

#endif
