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


#ifndef WIDGET_H
#define WIDGET_H

#include "settings.h"
#include <QWidget>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(const Settings& s);
    ~Widget();

    // Utilities
    void deleteBackups();
    void restoreBackups();
    void setProgress(int value);

    // Noreturn
    void fatalError(QString message); ///< Calls deleteUpdate and startQToxAndExit
    void deleteUpdate();
    void startQToxAndExit();

public slots:
    // Finds and applies the update
    void update();

private:
    Ui::Widget* ui;
    QStringList backups;
    const Settings& settings;
};

#endif // WIDGET_H
