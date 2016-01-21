/*
    Copyright © 2015 by The qTox Project

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

#include "contentlayout.h"
#include "src/persistence/settings.h"
#include "style.h"
#include <QStyleFactory>
#include <QFrame>

ContentLayout::ContentLayout()
    : QVBoxLayout()
{
    init();
}

ContentLayout::ContentLayout(QWidget *parent)
    : QVBoxLayout(parent)
{
    init();

    QPalette palette = parent->palette();
    palette.setBrush(QPalette::WindowText, QColor(0, 0, 0));
    palette.setBrush(QPalette::Button, QColor(255, 255, 255));
    palette.setBrush(QPalette::Light, QColor(255, 255, 255));
    palette.setBrush(QPalette::Midlight, QColor(255, 255, 255));
    palette.setBrush(QPalette::Dark, QColor(127, 127, 127));
    palette.setBrush(QPalette::Mid, QColor(170, 170, 170));
    palette.setBrush(QPalette::Text, QColor(0, 0, 0));
    palette.setBrush(QPalette::BrightText, QColor(255, 255, 255));
    palette.setBrush(QPalette::ButtonText, QColor(0, 0, 0));
    palette.setBrush(QPalette::Base, QColor(255, 255, 255));
    palette.setBrush(QPalette::Window, QColor(255, 255, 255));
    palette.setBrush(QPalette::Shadow, QColor(0, 0, 0));
    palette.setBrush(QPalette::AlternateBase, QColor(255, 255, 255));
    palette.setBrush(QPalette::ToolTipBase, QColor(255, 255, 220));
    palette.setBrush(QPalette::ToolTipText, QColor(0, 0, 0));

    palette.setBrush(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    palette.setBrush(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));

    parent->setPalette(palette);
}

ContentLayout::~ContentLayout()
{
    clear();

    mainHead->deleteLater();
    mainContent->deleteLater();
}

void ContentLayout::clear()
{
    QLayoutItem* item;
    while ((item = mainHead->layout()->takeAt(0)) != 0)
    {
        item->widget()->hide();
        item->widget()->setParent(nullptr);
        delete item;
    }

    while ((item = mainContent->layout()->takeAt(0)) != 0)
    {
        item->widget()->hide();
        item->widget()->setParent(nullptr);
        delete item;
    }
}

void ContentLayout::init()
{
    setMargin(0);
    setSpacing(0);

    mainHead = new QWidget();
    mainHead->setLayout(new QVBoxLayout);
    mainHead->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    mainHead->layout()->setMargin(0);
    mainHead->layout()->setSpacing(0);
    mainHead->setMouseTracking(true);

    mainHLine.setFrameShape(QFrame::HLine);
    mainHLine.setFrameShadow(QFrame::Plain);
    QPalette palette = mainHLine.palette();
    palette.setBrush(QPalette::WindowText, QBrush(QColor(193, 193, 193)));
    palette.setBrush(QPalette::WindowText, QBrush(QColor(193, 193, 193)));
    mainHLine.setPalette(palette);

    mainContent = new QWidget();
    mainContent->setLayout(new QVBoxLayout);
    mainContent->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    if (QStyleFactory::keys().contains(Settings::getInstance().getStyle())
            && Settings::getInstance().getStyle() != "None")
    {
        mainHead->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
        mainContent->setStyle(QStyleFactory::create(Settings::getInstance().getStyle()));
    }

#ifndef Q_OS_MAC
    mainHead->setStyleSheet(Style::getStylesheet(":ui/settings/mainHead.css"));
    mainContent->setStyleSheet(Style::getStylesheet(":ui/settings/mainContent.css"));
#endif

    mainHLineLayout.addWidget(&mainHLine);
    mainHLineLayout.addSpacing(5);

    addWidget(mainHead);
    addLayout(&mainHLineLayout);
    addWidget(mainContent);
}
