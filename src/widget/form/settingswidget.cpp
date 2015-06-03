/*
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
#include "src/video/camerasource.h"
#include "src/widget/form/settings/generalform.h"
#include "src/widget/form/settings/privacyform.h"
#include "src/widget/form/settings/avform.h"
#include "src/widget/form/settings/advancedform.h"
#include <QTabWidget>

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
{
    body = new QWidget(this);
    QVBoxLayout* bodyLayout = new QVBoxLayout();
    body->setLayout(bodyLayout);

    head = new QWidget(this);
    QHBoxLayout* headLayout = new QHBoxLayout();
    head->setLayout(headLayout);

    imgLabel = new QLabel();
    headLayout->addWidget(imgLabel);

    nameLabel = new QLabel();
    QFont bold;
    bold.setBold(true);
    nameLabel->setFont(bold);
    headLayout->addWidget(nameLabel);
    headLayout->addStretch(1);

    settingsWidgets = new QTabWidget(this);
    settingsWidgets->setTabPosition(QTabWidget::North);

    bodyLayout->addWidget(settingsWidgets);

    GeneralForm* gfrm = new GeneralForm(this);
    PrivacyForm* pfrm = new PrivacyForm;
    AVForm* avfrm = new AVForm;
    AdvancedForm *expfrm = new AdvancedForm;

    GenericForm* cfgForms[] = { gfrm, pfrm, avfrm, expfrm };
    for (GenericForm* cfgForm : cfgForms)
        settingsWidgets->addTab(cfgForm, cfgForm->getFormIcon(), cfgForm->getFormName());

    connect(settingsWidgets, &QTabWidget::currentChanged, this, &SettingsWidget::onTabChanged);
}

SettingsWidget::~SettingsWidget()
{
}

void SettingsWidget::setBodyHeadStyle(QString style)
{
    head->setStyle(QStyleFactory::create(style));    
    body->setStyle(QStyleFactory::create(style));
}

void SettingsWidget::show(Ui::MainWindow& ui)
{
    ui.mainContent->layout()->addWidget(body);
    ui.mainHead->layout()->addWidget(head);
    body->show();
    head->show();
    onTabChanged(settingsWidgets->currentIndex());
}

void SettingsWidget::onTabChanged(int index)
{
    this->settingsWidgets->setCurrentIndex(index);
    GenericForm* currentWidget = static_cast<GenericForm*>(this->settingsWidgets->widget(index));
    currentWidget->present();
    nameLabel->setText(currentWidget->getFormName());
    imgLabel->setPixmap(currentWidget->getFormIcon().scaledToHeight(40, Qt::SmoothTransformation));
}
