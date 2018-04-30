/*
    Copyright © 2014-2018 by The qTox Project Contributors

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

#include <QDebug>
#include <QTimer>
#include <tox/tox.h>

#include "src/core/recursivesignalblocker.h"
#include "src/net/autoupdate.h"
#include "src/widget/translator.h"

/**
 * @class AboutForm
 *
 * This form contains information about qTox and libraries versions, external
 * links and licence text. Shows progress during an update.
 */

/**
 * @brief Constructor of AboutForm.
 */
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

#if AUTOUPDATE_ENABLED
    showUpdateProgress();
    progressTimer->setInterval(500);
    progressTimer->setSingleShot(false);
    connect(progressTimer, &QTimer::timeout, this, &AboutForm::showUpdateProgress);
#else
    bodyUI->updateProgress->setVisible(false);
    bodyUI->updateText->setVisible(false);
#endif

    eventsInit();
    Translator::registerHandler(std::bind(&AboutForm::retranslateUi, this), this);
}

/**
 * @brief Update versions and links.
 *
 * Update commit hash if built with git, show author and known issues info
 * It also updates qTox, toxcore and Qt versions.
 */
void AboutForm::replaceVersions()
{
    // TODO: When we finally have stable releases: build-in a way to tell
    // nightly builds from stable releases.

    QString TOXCORE_VERSION = QString::number(tox_version_major()) + "."
                              + QString::number(tox_version_minor()) + "."
                              + QString::number(tox_version_patch());

    bodyUI->youAreUsing->setText(tr("You are using qTox version %1.").arg(QString(GIT_DESCRIBE)));

    QString commitLink = "https://github.com/qTox/qTox/commit/" + QString(GIT_VERSION);
    bodyUI->gitVersion->setText(
        tr("Commit hash: %1").arg(createLink(commitLink, QString(GIT_VERSION))));

    bodyUI->toxCoreVersion->setText(tr("toxcore version: %1").arg(TOXCORE_VERSION));
    bodyUI->qtVersion->setText(tr("Qt version: %1").arg(QT_VERSION_STR));

    QString issueBody = QString("##### Brief Description\n\n"
                                "OS: %1\n"
                                "qTox version: %2\n"
                                "Commit hash: %3\n"
                                "toxcore: %4\n"
                                "Qt: %5\n"
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
                            .arg(QSysInfo::prettyProductName(),  GIT_DESCRIBE, GIT_VERSION,
                                 TOXCORE_VERSION, QT_VERSION_STR);

    issueBody.replace("#", "%23").replace(":", "%3A");

    bodyUI->knownIssues->setText(
        tr("A list of all known issues may be found at our %1 at Github."
           " If you discover a bug or security vulnerability within"
           " qTox, please report it according to the guidelines in our"
           " %2 wiki article.",

           "`%1` is replaced by translation of `bug tracker`"
           "\n`%2` is replaced by translation of `Writing Useful Bug Reports`")
            .arg(createLink("https://github.com/qTox/qTox/issues",
                            tr("bug-tracker", "Replaces `%1` in the `A list of all known…`")))
            .arg(createLink("https://github.com/qTox/qTox/wiki/Writing-Useful-Bug-Reports",
                            tr("Writing Useful Bug Reports",
                               "Replaces `%2` in the `A list of all known…`"))));

    bodyUI->clickToReport->setText(
        createLink("https://github.com/qTox/qTox/issues/new?body=" + QUrl(issueBody).toEncoded(),
                   QString("<b>%1</b>").arg(tr("Click here to report a bug."))));


    QString authorInfo =
        QString("<p>%1</p><p>%2</p>")
            .arg(tr("Original author: %1").arg(createLink("https://github.com/tux3", "tux3")))
            .arg(
                tr("See a full list of %1 at Github",
                   "`%1` is replaced with translation of word `contributors`")
                    .arg(createLink("https://qtox.github.io/gitstats/authors.html",
                                    tr("contributors", "Replaces `%1` in `See a full list of…`"))));

    bodyUI->authorInfo->setText(authorInfo);
}

/**
 * @brief Creates hyperlink with specific style.
 * @param path The URL of the page the link goes to.
 * @param text Text, which will be clickable.
 * @return Hyperlink to paste.
 */
QString AboutForm::createLink(QString path, QString text) const
{
    return QString::fromUtf8(
               "<a href=\"%1\" style=\"text-decoration: underline; color:#0000ff;\">%2</a>")
        .arg(path, text);
}

AboutForm::~AboutForm()
{
    Translator::unregister(this);
    delete bodyUI;
}

/**
 * @brief Update information about update.
 */
void AboutForm::showUpdateProgress()
{
#if AUTOUPDATE_ENABLED
    QString version = AutoUpdater::getProgressVersion();
    int value = AutoUpdater::getProgressValue();

    if (version.isEmpty()) {
        bodyUI->updateProgress->setVisible(value != 0);
        bodyUI->updateText->setVisible(value != 0);
    } else {
        if (value == 100)
            bodyUI->updateText->setText(tr("Restart qTox to install version %1").arg(version));
        else
            bodyUI->updateText->setText(
                tr("qTox is downloading update %1", "%1 is the version of the update").arg(version));
        bodyUI->updateProgress->setValue(value);

        bodyUI->updateProgress->setVisible(value != 0 && value != 100);
        bodyUI->updateText->setVisible(value != 0);
    }
#endif
}

void AboutForm::hideEvent(QHideEvent*)
{
#if AUTOUPDATE_ENABLED
    progressTimer->stop();
#endif
}

void AboutForm::showEvent(QShowEvent*)
{
#if AUTOUPDATE_ENABLED
    progressTimer->start();
#endif
}

/**
 * @brief Retranslate all elements in the form.
 */
void AboutForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
    replaceVersions();
    showUpdateProgress();
}
