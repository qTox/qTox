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


#ifndef TOXURI_H
#define TOXURI_H

#include <QDialog>

/// Shows a dialog asking whether or not to add this tox address as a friend
/// Will wait until the core is ready first
bool handleToxURI(const QString& toxURI);

// Internals
class QByteArray;
class QPlainTextEdit;
bool toxURIEventHandler(const QByteArray& eventData);
class ToxURIDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ToxURIDialog(QWidget *parent, const QString &userId, const QString &message);
    QString getRequestMessage();

private:
    QPlainTextEdit *messageEdit;
};

#endif // TOXURI_H
