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
#include "src/widget/widget.h"
#include "ui_mainwindow.h"
#include "src/widget/camera.h"
#include "src/widget/form/settings/generalform.h"
#include "src/widget/form/settings/identityform.h"
#include "src/widget/form/settings/privacyform.h"
#include "src/widget/form/settings/avform.h"
#include <QTabBar>
#include <QStackedWidget>

SettingsWidget::SettingsWidget(Camera* cam, QWidget* parent)
    : QWidget(parent)
{
    body = new QWidget(this);
    QVBoxLayout *bodyLayout = new QVBoxLayout();
    body->setLayout(bodyLayout);

    head = new QWidget(this);
    QHBoxLayout *headLayout = new QHBoxLayout();
    head->setLayout(headLayout);

    imgLabel = new QLabel();
    headLayout->addWidget(imgLabel);

    nameLabel = new QLabel();
    QFont bold;
    bold.setBold(true);
    nameLabel->setFont(bold);
    headLayout->addWidget(nameLabel);
    headLayout->addStretch(1);

    settingsWidgets = new QStackedWidget;
    bodyLayout->addWidget(settingsWidgets);

    tabBar = new QTabBar;
    bodyLayout->addWidget(tabBar);

    GeneralForm *gfrm = new GeneralForm;
    ifrm = new IdentityForm;
    PrivacyForm *pfrm = new PrivacyForm;
    AVForm *avfrm = new AVForm(cam);

    GenericForm *cfgForms[] = {gfrm, ifrm, pfrm, avfrm};
    for (auto cfgForm : cfgForms)
    {
        tabBar->addTab(cfgForm->getFormIcon(), "");
        settingsWidgets->addWidget(cfgForm);
    }
    tabBar->setIconSize(QSize(20, 20));
    tabBar->setShape(QTabBar::RoundedSouth);

    connect(tabBar, &QTabBar::currentChanged, this, &SettingsWidget::onTabChanged);
}

SettingsWidget::~SettingsWidget()
{
}

void SettingsWidget::show(Ui::MainWindow& ui)
{
    ui.mainContent->layout()->addWidget(body);
    ui.mainHead->layout()->addWidget(head);
    body->show();
    head->show();
    onTabChanged(tabBar->currentIndex());
}

void SettingsWidget::onTabChanged(int index)
{
    this->settingsWidgets->setCurrentIndex(index);
    GenericForm *currentWidget = static_cast<GenericForm*>(this->settingsWidgets->widget(index));
    currentWidget->updateContent();
    nameLabel->setText(currentWidget->getFormName());
    imgLabel->setPixmap(currentWidget->getFormIcon().scaledToHeight(40, Qt::SmoothTransformation));
}
