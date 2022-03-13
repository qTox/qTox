/*
    Copyright © 2022 by The qTox Project Contributors

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

class QString;
class QFileInfo;

class IMessageBoxManager
{
public:
    virtual ~IMessageBoxManager();
    virtual void showInfo(const QString& title, const QString& msg) = 0;
    virtual void showWarning(const QString& title, const QString& msg) = 0;
    virtual void showError(const QString& title, const QString& msg) = 0;
    virtual bool askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
            bool warning = true, bool yesno = true) = 0;
    virtual bool askQuestion(const QString& title, const QString& msg, const QString& button1,
            const QString& button2, bool defaultAns = false, bool warning = true) = 0;
    virtual void confirmExecutableOpen(const QFileInfo& file) = 0;
};
