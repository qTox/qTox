/*
    Copyright © 2014-2016 by The qTox Project

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

#include "aboutform.h"
#include "ui_aboutsettings.h"

#include <QTimer>
#include <QDebug>
#include <tox/tox.h>

#include "src/core/recursivesignalblocker.h"
#include "src/net/autoupdate.h"
#include "src/widget/translator.h"

AboutForm::AboutForm()
    : GenericForm(QPixmap(":/img/settings/general.png"))
    , bodyUI(new Ui::AboutSettings)
    , progressTimer(new QTimer(this))
{
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    replaceVersions();

    if (QString(GIT_VERSION).indexOf(" ") > -1)
        bodyUI->gitVersion->setOpenExternalLinks(false);

    showUpdateProgress();
    progressTimer->setInterval(500);
    progressTimer->setSingleShot(false);
    connect(progressTimer, &QTimer::timeout, this, &AboutForm::showUpdateProgress);

    eventsInit();
    Translator::registerHandler(std::bind(&AboutForm::retranslateUi, this), this);
}

void AboutForm::replaceVersions()
{
    // TODO: When we finally have stable releases: build-in a way to tell
    // nightly builds from stable releases.

    QString TOXCORE_VERSION = QString::number(TOX_VERSION_MAJOR) + "." +
            QString::number(TOX_VERSION_MINOR) + "." +
            QString::number(TOX_VERSION_PATCH);

    bodyUI->youareusing->setText(bodyUI->youareusing->text().replace("$GIT_DESCRIBE", QString(GIT_DESCRIBE)));
    bodyUI->gitVersion->setText(bodyUI->gitVersion->text().replace("$GIT_VERSION", QString(GIT_VERSION)));
    bodyUI->toxCoreVersion->setText(bodyUI->toxCoreVersion->text().replace("$TOXCOREVERSION", TOXCORE_VERSION));
    bodyUI->qtVersion->setText(bodyUI->qtVersion->text().replace("$QTVERSION", QT_VERSION_STR));
    bodyUI->knownIssues->setText(
                tr("A list of all known issues may be found at our %1 at Github."
                   " If you discover a bug or security vulnerability within"
                   " qTox, please %3 according to the guidelines in our %2"
                   " wiki article.")
                .arg(QString::fromUtf8("<a href=\"https://github.com/qTox/qTox/"
                                       "issues\""
                                       " style=\"text-decoration: underline;"
                                       " color:#0000ff;\">%1</a>")
                     .arg(tr("bug-tracker")))
                .arg(QString::fromUtf8("<a href=\"https://github.com/qTox/qTox/"
                                       "wiki/Writing-Useful-Bug-Reports\""
                                       " style=\"text-decoration: underline;"
                                       " color:#0000ff;\">%1</a>")
                     .arg(tr("Writing Useful Bug Reports")))
                .arg(QStringLiteral(
                         "<a href=\"https://github.com/qTox/qTox/issues"
                         "/new?body=%23%23%23%23%23+Brief+Description%0A%0AOS"
                         "%3A+Windows+%2F+OS+X+%2F+Linux+(include+version+and"
                         "%2For+distro)%0AqTox+version%3A+") +
                     QStringLiteral(GIT_DESCRIBE) +
                     QStringLiteral("%0ACommit+hash%3A+") +
                     QStringLiteral(GIT_VERSION) +
                     QStringLiteral("%0Atoxcore%3A+") + TOXCORE_VERSION +
                     QStringLiteral("%0AQt%3A+") +
                     QStringLiteral(QT_VERSION_STR) +
                     QStringLiteral("%0AHardware%3A++%0A%E2%80%A6%0A%0A"
                                    "Reproducible%3A+Always+%2F+Almost+Always+"
                                    "%2F+Sometimes+%2F+Rarely+%2F+Couldn%27t+"
                                    "Reproduce%0A%0A%23%23%23%23%23+Steps+to+"
                                    "reproduce%0A%0A1.+%0A2.+%0A3.+%E2%80%A6"
                                    "%0A%0A%23%23%23%23%23+Observed+Behavior"
                                    "%0A%0A%0A%23%23%23%23%23+Expected+Behavior"
                                    "%0A%0A%0A%23%23%23%23%23+Additional+Info"
                                    "%0A(links%2C+images%2C+etc+go+here)%0A%0A"
                                    "----%0A%0AMore+information+on+how+to+"
                                    "write+good+bug+reports+in+the+wiki%3A+"
                                    "https%3A%2F%2Fgithub.com%2FqTox%2FqTox%2F"
                                    "wiki%2FWriting-Useful-Bug-Reports.%0A%0A"
                                    "Please+remove+any+unnecessary+template+"
                                    "section+before+submitting.\""
                                    " style=\"text-decoration: underline;"
                                    " color:#0000ff;\">") + tr("report it") +
                     QStringLiteral("</a>")
                     )
                );
}

AboutForm::~AboutForm()
{
    Translator::unregister(this);
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
