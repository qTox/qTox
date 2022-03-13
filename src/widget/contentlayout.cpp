/*
    Copyright © 2015-2019 by The qTox Project Contributors

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
#include "style.h"
#include "src/persistence/settings.h"
#include <QFrame>
#include <QStyleFactory>

ContentLayout::ContentLayout(Settings& settings_, Style& style_)
    : QVBoxLayout()
    , settings{settings_}
    , style{style_}
{
    init();
}

ContentLayout::ContentLayout(Settings& settings_, Style& style_, QWidget* parent)
    : QVBoxLayout(parent)
    , settings{settings_}
    , style{style_}
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

void ContentLayout::reloadTheme()
{
#ifndef Q_OS_MAC
    mainHead->setStyleSheet(style.getStylesheet("settings/mainHead.css", settings));
    mainContent->setStyleSheet(style.getStylesheet("window/general.css", settings));
#endif
}

void ContentLayout::clear()
{
    QLayoutItem* item;
    while ((item = mainHead->layout()->takeAt(0)) != nullptr) {
        item->widget()->hide();
        item->widget()->setParent(nullptr);
        delete item;
    }

    while ((item = mainContent->layout()->takeAt(0)) != nullptr) {
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
    mainHLine.setPalette(palette);

    mainContent = new QWidget();
    mainContent->setLayout(new QVBoxLayout);
    mainContent->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    if (QStyleFactory::keys().contains(settings.getStyle())
        && settings.getStyle() != "None") {
        mainHead->setStyle(QStyleFactory::create(settings.getStyle()));
        mainContent->setStyle(QStyleFactory::create(settings.getStyle()));
    }

    connect(&style, &Style::themeReload, this, &ContentLayout::reloadTheme);

    reloadTheme();

    mainHLineLayout.addSpacing(4);
    mainHLineLayout.addWidget(&mainHLine);
    mainHLineLayout.addSpacing(5);

    addWidget(mainHead);
    addLayout(&mainHLineLayout);
    addWidget(mainContent);
}
