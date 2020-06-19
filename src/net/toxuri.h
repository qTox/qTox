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

#include <QDialog>

class Core;
// Internals
class QByteArray;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class ToxURIDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ToxURIDialog(QWidget* parent, Core& _core);
    QString getRequestMessage();
    bool handleToxURI(const QString& toxURI);
    bool toxURIEventHandler(const QByteArray& eventData);

private:
    void setUserId(const QString& userId);

private:
    QPlainTextEdit* messageEdit;
    QLabel* friendsLabel;
    QLineEdit* userIdEdit;
    Core& core;
};
