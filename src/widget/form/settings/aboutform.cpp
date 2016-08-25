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

    QString issueBody = QString(
            "##### Brief Description\n\n"
            "OS: Windows / OS X / Linux (include version and/or distro)\n"
            "qTox version: %1\n"
            "Commit hash: %2\n"
            "toxcore: %3\n"
            "Qt: %4\n"
            "Hardware: \n…\n\n"
            "Reproducible: Always / Almost Always / Sometimes"
            " / Rarely / Couldn't Reproduce\n\n"
            "##### Steps to reproduce\n\n"
            "1. \n2. \n3. …\n\n"
            "##### Observed Behavior\n\n\n"
            "##### Expected Behavior\n\n\n"
            "##### Additional Info\n"
            "(links, images, etc go here)\n\n"
            "----\n\n"
            "More information on how to write good bug reports in the wiki: "
            "https://github.com/qTox/qTox/wiki/Writing-Useful-Bug-Reports.\n\n"
            "Please remove any unnecessary template section before submitting.")
            .arg(GIT_DESCRIBE, GIT_VERSION, TOXCORE_VERSION, QT_VERSION_STR);

    issueBody.replace("#", "%23").replace(":", "%3A");

    bodyUI->knownIssues->setText(
                tr("A list of all known issues may be found at our %1 at Github."
                   " If you discover a bug or security vulnerability within"
                   " qTox, please %3 according to the guidelines in our %2"
                   " wiki article.")
                .arg(createLink("https://github.com/qTox/qTox/issues",
                     tr("bug-tracker")))
                .arg(createLink("https://github.com/qTox/qTox/wiki/Writing-Useful-Bug-Reports",
                     tr("Writing Useful Bug Reports")))
                .arg(createLink("https://github.com/qTox/qTox/issues/new?body="
                                + QUrl(issueBody).toEncoded(),
                     tr("report it")))
                );


    QString authorInfo = QString("<p>%1</p><p>%2</p>")
            .arg(tr("Original author: %1")
                 .arg(createLink("https://github.com/tux3", "tux3")))
            .arg(tr("See a full list of %1 at Github")
                 .arg(createLink("https://github.com/qTox/qTox/graphs/contributors",
                                 tr("contributors"))));

    bodyUI->authorInfo->setText(authorInfo);
}

QString AboutForm::createLink(QString path, QString text) const
{
    return QString::fromUtf8("<a href=\"%1\" style=\"text-decoration: underline; color:#0000ff;\">%2</a>")
            .arg(path, text);
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
