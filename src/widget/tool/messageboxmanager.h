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

#include <QObject>
#include <QWidget>

#include "src/widget/tool/imessageboxmanager.h"

class QFileInfo;

class MessageBoxManager : public QWidget, public IMessageBoxManager
{
Q_OBJECT
public:
    explicit MessageBoxManager(QWidget* parent);
    ~MessageBoxManager() override = default;
    void showInfo(const QString& title, const QString& msg) override;
    void showWarning(const QString& title, const QString& msg) override;
    void showError(const QString& title, const QString& msg) override;
    bool askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
                            bool warning = true, bool yesno = true) override;
    bool askQuestion(const QString& title, const QString& msg, const QString& button1,
                            const QString& button2, bool defaultAns = false, bool warning = true) override;
    void confirmExecutableOpen(const QFileInfo& file) override;

private slots:
    // Private implementation, those must be called from the GUI thread
    void _showInfo(const QString& title, const QString& msg);
    void _showWarning(const QString& title, const QString& msg);
    void _showError(const QString& title, const QString& msg);
    bool _askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
                      bool warning = true, bool yesno = true);
    bool _askQuestion(const QString& title, const QString& msg, const QString& button1,
                      const QString& button2, bool defaultAns = false, bool warning = true);
};
