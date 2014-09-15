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

#include "generalform.h"
#include "widget/form/settingswidget.h"

GeneralForm::GeneralForm()
{
    prep();
    icon.setPixmap(QPixmap(":/img/settings/general.png").scaledToHeight(headLayout.sizeHint().height(), Qt::SmoothTransformation));
    label.setText(tr("General settings"));
    group = new QGroupBox(tr("General Settings"));
    enableIPv6 = new QCheckBox();
    enableIPv6->setText(tr("Enable IPv6 (recommended)","Text on a checkbox to enable IPv6"));
    useTranslations = new QCheckBox();
    useTranslations->setText(tr("Use translations","Text on a checkbox to enable translations"));
    makeToxPortable = new QCheckBox();
    makeToxPortable->setText(tr("Make Tox portable","Text on a checkbox to make qTox a portable application"));
    makeToxPortable->setToolTip(tr("Save settings to the working directory instead of the usual conf dir","describes makeToxPortable checkbox"));

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(enableIPv6);
    vLayout->addWidget(useTranslations);
    vLayout->addWidget(makeToxPortable);
    group->setLayout(vLayout);
    
    label.setText(tr("General Settings"));
    
    headLayout.addWidget(&label);
    layout.addWidget(group);
}

GeneralForm::~GeneralForm()
{
}
