/*
    Copyright © 2019-2020 by The qTox Project Contributors

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

#include "extensionstatus.h"

#include <QIcon>

ExtensionStatus::ExtensionStatus(QWidget* parent)
    : QLabel(parent)
{
    // Initialize with 0 extensions
    onExtensionSetUpdate(ExtensionSet());
}

void ExtensionStatus::onExtensionSetUpdate(ExtensionSet extensionSet)
{
    QString iconName;
    QString hoverText;
    if (extensionSet.all()) {
        iconName = ":/img/status/extensions_available.svg";
        hoverText = tr("All extensions supported");
    } else if (extensionSet.none()) {
        iconName = ":/img/status/extensions_unavailable.svg";
        hoverText = tr("No extensions supported");
    } else {
        iconName = ":/img/status/extensions_partial.svg";
        hoverText = tr("Not all extensions supported");
    }

    hoverText += "\n";
    hoverText += tr("Multipart Messages: ");
    hoverText += extensionSet[ExtensionType::messages] ? "✔" : "❌";

    auto pixmap = QIcon(iconName).pixmap(QSize(16, 16));

    setPixmap(pixmap);
    setToolTip(hoverText);
}
