/*
    Copyright © 2018 by The qTox Project Contributors

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

#include "upgradablesetting.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

std::vector<bool> UpgradableSettingDetail::doUpgradeRequest(QString const& message,
                                                            std::vector<MessageItem> messageItems)
{
    if (messageItems.size() == 0) {
        return {};
    } else if (messageItems.size() == 1) {
        QString fullMessage = message;
        fullMessage += "\nOld value: %1\nNew default: %2";
        fullMessage = fullMessage.arg(messageItems[0].oldValue).arg(messageItems[0].newDefault);
        auto response = QMessageBox::question(nullptr, QObject::tr("Upgrade Request"), fullMessage);
        return {response == QMessageBox::StandardButton::Yes};
    } else {
        QDialog upgradeDialog;
        upgradeDialog.setWindowTitle(QObject::tr("Upgrade Request"));
        upgradeDialog.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
        QVBoxLayout layout(&upgradeDialog);

        QLabel messageLabel(message);
        messageLabel.setWordWrap(true);
        layout.addWidget(&messageLabel);

        QScrollArea scrollArea;
        QWidget itemsWidget;
        QGridLayout itemsLayout(&itemsWidget);
        layout.addWidget(&scrollArea);
        scrollArea.setWidget(&itemsWidget);
        scrollArea.setWidgetResizable(true);
        scrollArea.horizontalScrollBar()->setEnabled(false);
        scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        auto nameHeader = new QLabel(QObject::tr("Name"));
        itemsLayout.addWidget(nameHeader, 0, 0);

        auto oldHeader = new QLabel(QObject::tr("Old Value"));
        itemsLayout.addWidget(oldHeader, 0, 1);

        auto newHeader = new QLabel(QObject::tr("New Default"));
        itemsLayout.addWidget(newHeader, 0, 2);

        auto acceptHeader = new QLabel(QObject::tr("Accept"));
        itemsLayout.addWidget(acceptHeader, 0, 3);

        int rowIdx = 1;

        std::vector<QCheckBox*> checkboxes;

        for (auto const& messageItem : messageItems) {
            auto nameLabel = new QLabel(messageItem.name);
            itemsLayout.addWidget(nameLabel, rowIdx, 0);

            auto oldValueLabel = new QLabel(messageItem.oldValue);
            oldValueLabel->setWordWrap(true);
            itemsLayout.addWidget(oldValueLabel, rowIdx, 1);

            auto newValueLabel = new QLabel(messageItem.newDefault);
            newValueLabel->setWordWrap(true);
            itemsLayout.addWidget(newValueLabel, rowIdx, 2);

            auto checkbox = new QCheckBox();
            checkbox->setCheckState(Qt::CheckState::Checked);
            checkboxes.push_back(checkbox);
            itemsLayout.addWidget(checkbox, rowIdx, 3);

            rowIdx++;
        }

        scrollArea.setMinimumWidth(itemsLayout.sizeHint().width());

        QHBoxLayout upgradeAllHbox;
        layout.addLayout(&upgradeAllHbox);

        QLabel upgradeAllLabel(QObject::tr("Upgrade all: "));
        upgradeAllHbox.addWidget(&upgradeAllLabel);

        QSizePolicy upgradeAllLabelSizePolicy;
        upgradeAllLabelSizePolicy.setHorizontalPolicy(QSizePolicy::Policy::Expanding);
        upgradeAllLabel.setSizePolicy(upgradeAllLabelSizePolicy);

        auto upgradeAllCheckbox = new QCheckBox();
        upgradeAllHbox.addWidget(upgradeAllCheckbox);

        upgradeAllCheckbox->setChecked(true);
        QCheckBox::connect(upgradeAllCheckbox, &QCheckBox::stateChanged, [&](int state) {
            for (auto& checkbox : checkboxes) {
                checkbox->setCheckState(static_cast<Qt::CheckState>(state));
            }
        });

        QDialogButtonBox buttonBox;
        QPushButton okButton;
        okButton.setText(QObject::tr("OK"));
        QObject::connect(&okButton, &QPushButton::clicked, &upgradeDialog, &QDialog::close);
        buttonBox.addButton(&okButton, QDialogButtonBox::AcceptRole);
        layout.addWidget(&buttonBox);

        upgradeDialog.exec();

        std::vector<bool> successes;
        successes.reserve(checkboxes.size());
        std::transform(checkboxes.begin(), checkboxes.end(), std::back_inserter(successes),
                       [](QCheckBox* checkbox) { return checkbox->isChecked(); });

        return successes;
    }
}
