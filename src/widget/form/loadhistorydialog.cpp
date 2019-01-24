/*
    Copyright © 2014-2018 by The qTox Project Contributors

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

#include "loadhistorydialog.h"
#include "ui_loadhistorydialog.h"
#include "src/persistence/history.h"
#include "src/persistence/settings.h"
#include <QDate>
#include <QLabel>
#include <QTextCharFormat>
#include <QCalendarWidget>

LoadHistoryDialog::LoadHistoryDialog(const ToxPk& friendPk, History &history, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoadHistoryDialog)
    , friendPk(friendPk)
    , history{history}
{
    ui->setupUi(this);
    getYears();
    ui->yearsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

LoadHistoryDialog::LoadHistoryDialog(History &history, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoadHistoryDialog)
    , history{history}
{
    ui->setupUi(this);
}

LoadHistoryDialog::~LoadHistoryDialog()
{
    delete ui;
}

QDateTime LoadHistoryDialog::getFromDate()
{
    auto selection = ui->yearsTree->selectedItems();
    if(selection.size() == 0) {
        qDebug() << "No element selected, returning default value";
        return{};
    }

    return selection[0]->data(0, Qt::UserRole).toDateTime();
}

void LoadHistoryDialog::setTitle(const QString& title)
{
    setWindowTitle(title);
}

void LoadHistoryDialog::setInfoLabel(const QString& info)
{
    ui->fromLabel->setText(info);
}

void LoadHistoryDialog::getYears()
{
    auto counts = history.getChatHistoryYears(this->friendPk);

    auto tree = ui->yearsTree;

    for (auto count : counts) {
        QTreeWidgetItem* year = new QTreeWidgetItem(tree);

        const Qt::ItemFlags newFlags = year->flags() &~ Qt::ItemIsSelectable;
        year->setFlags(newFlags);

        year->setData(0, Qt::DisplayRole, count.year);
        year->setData(1, Qt::DisplayRole, count.count);
        tree->addTopLevelItem(year);

        const QDate start(count.year, 1, 1);
        const QDate end = start.addYears(1);
        QList<History::DateMessages> date_counts = history.getChatHistoryCounts(this->friendPk, start, end);

        for(auto elem: date_counts) {
            QTreeWidgetItem* dayItem = new QTreeWidgetItem(year);
            QDate exact = start.addDays(elem.offsetDays);
            dayItem->setData(0, Qt::DisplayRole, exact.toString(Settings::getInstance().getDateFormat()));
            dayItem->setData(1, Qt::DisplayRole, elem.count);
            dayItem->setData(0, Qt::UserRole, exact);   // store date in machine form here
        }
    }
}
