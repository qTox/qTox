/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "settingswidget.h"
#include "widget/widget.h"
#include "ui_mainwindow.h"

SettingsWidget::SettingsWidget()
    : QWidget()
{
    _main = new QWidget();
    main = new QWidget();
    head = new QWidget();
    foot = new QWidget();
    prepButtons();    
    foot->setLayout(iconsLayout);
    _mainLayout = new QVBoxLayout(_main);
    _mainLayout->addWidget(main);
    _mainLayout->addWidget(foot);
    // something something foot size
    _main->setLayout(_mainLayout);
}

SettingsWidget::~SettingsWidget()
{
}

void SettingsWidget::show(Ui::MainWindow& ui)
{
    ui.mainContent->layout()->addWidget(_main);
    ui.mainHead->layout()->addWidget(head);
    _main->show();
    head->show();
}

void SettingsWidget::prepButtons()
{
    // this crap is copied from ui_mainwindow.h... there's no easy way around
    // just straight up copying it like this... oh well
    // the layout/icons obviously need to be improved, but it's a working model,
    // not a pretty one
    QSizePolicy sizePolicy3(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    foot->setObjectName(QStringLiteral("foot"));
    foot->setEnabled(true);
    foot->setSizePolicy(sizePolicy3);
    QPalette palette5;
    QBrush brush(QColor(255, 255, 255, 255));
    brush.setStyle(Qt::SolidPattern);
    QBrush brush1(QColor(28, 28, 28, 255));
    brush1.setStyle(Qt::SolidPattern);
    QBrush brush2(QColor(42, 42, 42, 255));
    brush2.setStyle(Qt::SolidPattern);
    QBrush brush3(QColor(35, 35, 35, 255));
    brush3.setStyle(Qt::SolidPattern);
    QBrush brush4(QColor(14, 14, 14, 255));
    brush4.setStyle(Qt::SolidPattern);
    QBrush brush5(QColor(18, 18, 18, 255));
    brush5.setStyle(Qt::SolidPattern);
    QBrush brush6(QColor(0, 0, 0, 255));
    brush6.setStyle(Qt::SolidPattern);
    QBrush brush7(QColor(255, 255, 220, 255));
    brush7.setStyle(Qt::SolidPattern);
    palette5.setBrush(QPalette::Active, QPalette::WindowText, brush);
    palette5.setBrush(QPalette::Active, QPalette::Button, brush1);
    palette5.setBrush(QPalette::Active, QPalette::Light, brush2);
    palette5.setBrush(QPalette::Active, QPalette::Midlight, brush3);
    palette5.setBrush(QPalette::Active, QPalette::Dark, brush4);
    palette5.setBrush(QPalette::Active, QPalette::Mid, brush5);
    palette5.setBrush(QPalette::Active, QPalette::Text, brush);
    palette5.setBrush(QPalette::Active, QPalette::BrightText, brush);
    palette5.setBrush(QPalette::Active, QPalette::ButtonText, brush);
    palette5.setBrush(QPalette::Active, QPalette::Base, brush6);
    palette5.setBrush(QPalette::Active, QPalette::Window, brush1);
    palette5.setBrush(QPalette::Active, QPalette::Shadow, brush6);
    palette5.setBrush(QPalette::Active, QPalette::AlternateBase, brush4);
    palette5.setBrush(QPalette::Active, QPalette::ToolTipBase, brush7);
    palette5.setBrush(QPalette::Active, QPalette::ToolTipText, brush6);
    palette5.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
    palette5.setBrush(QPalette::Inactive, QPalette::Button, brush1);
    palette5.setBrush(QPalette::Inactive, QPalette::Light, brush2);
    palette5.setBrush(QPalette::Inactive, QPalette::Midlight, brush3);
    palette5.setBrush(QPalette::Inactive, QPalette::Dark, brush4);
    palette5.setBrush(QPalette::Inactive, QPalette::Mid, brush5);
    palette5.setBrush(QPalette::Inactive, QPalette::Text, brush);
    palette5.setBrush(QPalette::Inactive, QPalette::BrightText, brush);
    palette5.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
    palette5.setBrush(QPalette::Inactive, QPalette::Base, brush6);
    palette5.setBrush(QPalette::Inactive, QPalette::Window, brush1);
    palette5.setBrush(QPalette::Inactive, QPalette::Shadow, brush6);
    palette5.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush4);
    palette5.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush7);
    palette5.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush6);
    palette5.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
    palette5.setBrush(QPalette::Disabled, QPalette::Button, brush1);
    palette5.setBrush(QPalette::Disabled, QPalette::Light, brush2);
    palette5.setBrush(QPalette::Disabled, QPalette::Midlight, brush3);
    palette5.setBrush(QPalette::Disabled, QPalette::Dark, brush4);
    palette5.setBrush(QPalette::Disabled, QPalette::Mid, brush5);
    palette5.setBrush(QPalette::Disabled, QPalette::Text, brush4);
    palette5.setBrush(QPalette::Disabled, QPalette::BrightText, brush);
    palette5.setBrush(QPalette::Disabled, QPalette::ButtonText, brush4);
    palette5.setBrush(QPalette::Disabled, QPalette::Base, brush1);
    palette5.setBrush(QPalette::Disabled, QPalette::Window, brush1);
    palette5.setBrush(QPalette::Disabled, QPalette::Shadow, brush6);
    palette5.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush1);
    palette5.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush7);
    palette5.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush6);
    foot->setPalette(palette5);
    foot->setAutoFillBackground(true);
    
    iconsLayout = new QHBoxLayout(foot);
    iconsLayout->setSpacing(0);
    iconsLayout->setObjectName(QStringLiteral("iconsLayout"));
    iconsLayout->setContentsMargins(0, 0, 0, 0);

    generalButton = new QPushButton(foot);
    generalButton->setObjectName(QStringLiteral("generalButton"));
    generalButton->setMinimumSize(QSize(55, 35));
    generalButton->setMaximumSize(QSize(55, 35));
    generalButton->setFocusPolicy(Qt::NoFocus);
    QIcon icon1;
    icon1.addFile(QStringLiteral(":/img/add.png"), QSize(), QIcon::Normal, QIcon::Off);
    generalButton->setIcon(icon1);
    generalButton->setFlat(true);
    iconsLayout->addWidget(generalButton);
    
    identityButton = new QPushButton(foot);
    identityButton->setObjectName(QStringLiteral("identityButton"));
    identityButton->setMinimumSize(QSize(55, 35));
    identityButton->setMaximumSize(QSize(55, 35));
    identityButton->setFocusPolicy(Qt::NoFocus);
    QIcon icon2;
    icon2.addFile(QStringLiteral(":/img/group.png"), QSize(), QIcon::Normal, QIcon::Off);
    identityButton->setIcon(icon2);
    identityButton->setFlat(true);
    iconsLayout->addWidget(identityButton);
    
    privacyButton = new QPushButton(foot);
    privacyButton->setObjectName(QStringLiteral("privacyButton"));
    privacyButton->setMinimumSize(QSize(55, 35));
    privacyButton->setMaximumSize(QSize(55, 35));
    privacyButton->setFocusPolicy(Qt::NoFocus);
    QIcon icon3;
    icon3.addFile(QStringLiteral(":/img/transfer.png"), QSize(), QIcon::Normal, QIcon::Off);
    privacyButton->setIcon(icon3);
    privacyButton->setFlat(true);
    iconsLayout->addWidget(privacyButton);
    
    avButton = new QPushButton(foot);
    avButton->setObjectName(QStringLiteral("avButton"));
    avButton->setMinimumSize(QSize(55, 35));
    avButton->setMaximumSize(QSize(55, 35));
    avButton->setFocusPolicy(Qt::NoFocus);
    QIcon icon4;
    icon4.addFile(QStringLiteral(":/img/settings.png"), QSize(), QIcon::Normal, QIcon::Off);
    avButton->setIcon(icon4);
    avButton->setFlat(true);
    iconsLayout->addWidget(avButton);
}
