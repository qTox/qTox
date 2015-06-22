/*
    Copyright © 2014-2015 by The qTox Project

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

#include "ui_aboutsettings.h"

#include "aboutform.h"
#include "src/widget/translator.h"

AboutForm::AboutForm() :
    GenericForm(QPixmap(":/img/settings/general.png"))
{
    bodyUI = new Ui::AboutSettings;
    bodyUI->setupUi(this);
    //to-do: when we finally have stable releases: build-in a way to tell
    //nightly builds from stable releases.
    bodyUI->label_4->setText(bodyUI->label_4->text().replace("GIT_VERSION", QString(GIT_VERSION)));

    Translator::registerHandler(std::bind(&AboutForm::retranslateUi, this), this);
}

AboutForm::~AboutForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void AboutForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
    bodyUI->label_4->setText(bodyUI->label_4->text().replace("GIT_VERSION", QString(GIT_VERSION)));
}
