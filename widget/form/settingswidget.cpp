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
#include "widget/camera.h"
#include "widget/form/settings/generalform.h"
#include "widget/form/settings/identityform.h"
#include "widget/form/settings/privacyform.h"
#include "widget/form/settings/avform.h"
#include <QTabWidget>

SettingsWidget::SettingsWidget(Camera* cam, QWidget* parent)
    : QWidget(parent)
{
    body = new QWidget();
    head = new QWidget();

    QVBoxLayout *bodyLayout = new QVBoxLayout();
    body->setLayout(bodyLayout);

    QTabWidget *settingsTabs = new QTabWidget();
    bodyLayout->addWidget(settingsTabs);

    GeneralForm *gfrm = new GeneralForm;
    ifrm = new IdentityForm;
    PrivacyForm *pfrm = new PrivacyForm;
    AVForm *avfrm = new AVForm(cam);

    GenericForm *cfgForms[] = {gfrm, ifrm, pfrm, avfrm};
    for (auto cfgForm : cfgForms)
    {
        settingsTabs->addTab(cfgForm, cfgForm->getFormIcon(), cfgForm->getFormName());
    }
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
}
