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
#include "src/net/autoupdate.h"
#include <QTimer>
#include <QDebug>

AboutForm::AboutForm() :
    GenericForm(QPixmap(":/img/settings/general.png"))
{
    bodyUI = new Ui::AboutSettings;
    bodyUI->setupUi(this);
    replaceVersions();

    if (QString(GIT_VERSION).indexOf(" ") > -1)
        bodyUI->gitVersion->setOpenExternalLinks(false);

    showUpdateProgress();
    progressTimer = new QTimer();
    progressTimer->setInterval(500);
    progressTimer->setSingleShot(false);
    connect(progressTimer, &QTimer::timeout, this, &AboutForm::showUpdateProgress);

    Translator::registerHandler(std::bind(&AboutForm::retranslateUi, this), this);
}

//to-do: when we finally have stable releases: build-in a way to tell
//nightly builds from stable releases.
void AboutForm::replaceVersions()
{
    QString TOXCORE_VERSION = QString::number(TOX_VERSION_MAJOR) + "." +
      QString::number(TOX_VERSION_MINOR) + "." +
      QString::number(TOX_VERSION_PATCH);
    bodyUI->youareusing->setText(bodyUI->youareusing->text().replace("$GIT_DESCRIBE", QString(GIT_DESCRIBE)));
    bodyUI->gitVersion->setText(bodyUI->gitVersion->text().replace("$GIT_VERSION", QString(GIT_VERSION)));
    bodyUI->toxCoreVersion->setText(bodyUI->toxCoreVersion->text().replace("$TOXCOREVERSION", TOXCORE_VERSION));
    bodyUI->qtVersion->setText(bodyUI->qtVersion->text().replace("$QTVERSION", QT_VERSION_STR));
    bodyUI->knownIssues->setText(
      tr("A list of all known issues may be found at our %1 at Github. If you discover a bug or security vulnerability within qTox, please %3 according to the guidelines in our %2 wiki article.")
        .arg(QString::fromUtf8("<a href=\"https://github.com/tux3/qTox/issues\" style=\"text-decoration: underline; color:#0000ff;\">%1</a>")
          .arg(tr("bug-tracker")))
        .arg(QString::fromUtf8("<a href=\"https://github.com/tux3/qTox/wiki/Writing-Useful-Bug-Reports\" style=\"text-decoration: underline; color:#0000ff;\">%1</a>")
          .arg(tr("Writing Useful Bug Reports")))
        .arg(QString::fromUtf8("<a href=\"https://github.com/tux3/qTox/issues/"
            "new?body=%23%23%23%23%23+Brief+Description%1A%1AOS%3A+Windows+%2F+"
            "OS+X+%2F+Linux+(include+version+and%2For+distro)%1AqTox+version"
            "%3A+%4%1ACommit+hash%3A+%5%1Atoxcore%3A+%6%1AQt%3A+%7%1A"
            "Hardware%3A++%1A%E2%80%A6%1A%1AReproducible%3A+Always+%2F+Almost+"
            "Always+%2F+Sometimes+%2F+Rarely+%2F+Couldn%27t+Reproduce%1A%1A%23"
            "%23%23%23%23+Steps+to+reproduce%1A%1A1.+%1A2.+%1A3.+%E2%80%A6%1A"
            "%1A%23%23%23%23%23+Observed+Behavior%1A%1A%1A%23%23%23%23%23+"
            "Expected+Behavior%1A%1A%1A%23%23%23%23%23+Additional+Info%1A(links"
            "%2C+images%2C+etc+go+here)%1A%1A----%1A%1AMore+information+on+how+"
            "to+write+good+bug+reports+in+the+wiki%3A+https%3A%2F%2Fgithub.com"
            "%2Ftux3%2FqTox%2Fwiki%2FWriting-Useful-Bug-Reports.%1A%1APlease+"
            "remove+any+unnecessary+template+section+before+submitting.\" "
            "style=\"text-decoration: underline; color:#0000ff;\">%8</a>")
          .arg(
              QString("%0"),
              QString("%2"),
              QString("%3"),
              QString(GIT_DESCRIBE),
              QString(GIT_VERSION),
              QString(TOXCORE_VERSION),
              QString(QT_VERSION_STR),
              tr("report it")))
    );
}

AboutForm::~AboutForm()
{
    Translator::unregister(this);
    delete progressTimer;
    delete bodyUI;
}

void AboutForm::showUpdateProgress()
{
    QString version = AutoUpdater::getProgressVersion();
    int value = AutoUpdater::getProgressValue();

    if (version.isEmpty())
    {
        bodyUI->updateProgress->setVisible(value != 0);
        bodyUI->updateText->setVisible(value != 0);
    }
    else
    {
        if (value == 100)
            bodyUI->updateText->setText(tr("Restart qTox to install version %1").arg(version));
        else
            bodyUI->updateText->setText(tr("qTox is downloading update %1", "%1 is the version of the update").arg(version));
        bodyUI->updateProgress->setValue(value);

        bodyUI->updateProgress->setVisible(value != 0 && value != 100);
        bodyUI->updateText->setVisible(value != 0);
    }
}

void AboutForm::hideEvent(QHideEvent *)
{
    progressTimer->stop();
}

void AboutForm::showEvent(QShowEvent *)
{
    progressTimer->start();
}

void AboutForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
    replaceVersions();
    showUpdateProgress();
}
