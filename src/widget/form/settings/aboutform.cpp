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
#include "tox/tox.h"

AboutForm::AboutForm() :
    GenericForm(QPixmap(":/img/settings/general.png"))
{
    bodyUI = new Ui::AboutSettings;
    bodyUI->setupUi(this);
    replaceVersions();

    if (QString(GIT_VERSION).indexOf(" ") > -1)
        bodyUI->gitVersion->setOpenExternalLinks(false);

    Translator::registerHandler(std::bind(&AboutForm::retranslateUi, this), this);
}

//to-do: when we finally have stable releases: build-in a way to tell
//nightly builds from stable releases.
void AboutForm::replaceVersions()
{
    bodyUI->gitVersion->setText(bodyUI->gitVersion->text().replace("$GIT_VERSION", QString(GIT_VERSION)));
    bodyUI->toxCoreVersion->setText(
                bodyUI->toxCoreVersion->text().replace("$TOXCOREVERSION",
                                                       QString::number(TOX_VERSION_MAJOR) + "." +
                                                       QString::number(TOX_VERSION_MINOR) + "." +
                                                       QString::number(TOX_VERSION_PATCH)));
    bodyUI->qtVersion->setText(
                bodyUI->qtVersion->text().replace("$QTVERSION", QT_VERSION_STR));
}

AboutForm::~AboutForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

void AboutForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
    replaceVersions();
}
