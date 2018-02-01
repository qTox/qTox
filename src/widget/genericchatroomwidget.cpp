/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

#include "genericchatroomwidget.h"
#include "maskablepixmapwidget.h"
#include "src/persistence/settings.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include <QBoxLayout>
#include <QMouseEvent>

GenericChatroomWidget::GenericChatroomWidget(bool compact, QWidget* parent)
    : GenericChatItemWidget(compact, parent)
    , active{false}
{
    // avatar
    QSize size;
    if (isCompact())
        size = QSize(20, 20);
    else
        size = QSize(40, 40);

    avatar = new MaskablePixmapWidget(this, size, ":/img/avatar_mask.svg");

    // status text
    statusMessageLabel = new CroppingLabel(this);
    statusMessageLabel->setTextFormat(Qt::PlainText);
    statusMessageLabel->setForegroundRole(QPalette::WindowText);

    nameLabel->setForegroundRole(QPalette::WindowText);

    Settings& s = Settings::getInstance();
    connect(&s, &Settings::compactLayoutChanged, this, &GenericChatroomWidget::compactChange);

    setAutoFillBackground(true);
    reloadTheme();

    compactChange(isCompact());
}

bool GenericChatroomWidget::eventFilter(QObject*, QEvent*)
{
    return true; // Disable all events.
}

void GenericChatroomWidget::compactChange(bool _compact)
{
    if (!isCompact())
        delete textLayout; // has to be first, deleted by layout

    setCompact(_compact);

    delete mainLayout;

    mainLayout = new QHBoxLayout;
    textLayout = new QVBoxLayout;

    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    textLayout->setSpacing(0);
    textLayout->setMargin(0);
    setLayoutDirection(Qt::LeftToRight); // parent might have set Qt::RightToLeft

    // avatar
    if (isCompact()) {
        delete textLayout; // Not needed
        setFixedHeight(25);
        avatar->setSize(QSize(20, 20));
        mainLayout->addSpacing(18);
        mainLayout->addWidget(avatar);
        mainLayout->addSpacing(5);
        mainLayout->addWidget(nameLabel);
        mainLayout->addWidget(statusMessageLabel);
        mainLayout->addSpacing(5);
        mainLayout->addWidget(&statusPic);
        mainLayout->addSpacing(5);
        mainLayout->activate();
        statusMessageLabel->setFont(Style::getFont(Style::Small));
        nameLabel->setFont(Style::getFont(Style::Medium));
    } else {
        setFixedHeight(55);
        avatar->setSize(QSize(40, 40));
        textLayout->addStretch();
        textLayout->addWidget(nameLabel);
        textLayout->addWidget(statusMessageLabel);
        textLayout->addStretch();
        mainLayout->addSpacing(20);
        mainLayout->addWidget(avatar);
        mainLayout->addSpacing(10);
        mainLayout->addLayout(textLayout);
        mainLayout->addSpacing(10);
        mainLayout->addWidget(&statusPic);
        mainLayout->addSpacing(10);
        mainLayout->activate();
        statusMessageLabel->setFont(Style::getFont(Style::Medium));
        nameLabel->setFont(Style::getFont(Style::Big));
    }
}

bool GenericChatroomWidget::isActive()
{
    return active;
}

void GenericChatroomWidget::setActive(bool _active)
{
    active = _active;
    if (active) {
        setBackgroundRole(QPalette::Light);
        statusMessageLabel->setForegroundRole(QPalette::HighlightedText);
        nameLabel->setForegroundRole(QPalette::HighlightedText);
    } else {
        setBackgroundRole(QPalette::Window);
        statusMessageLabel->setForegroundRole(QPalette::WindowText);
        nameLabel->setForegroundRole(QPalette::WindowText);
    }
}

void GenericChatroomWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}

void GenericChatroomWidget::setStatusMsg(const QString& status)
{
    statusMessageLabel->setText(status);
}

QString GenericChatroomWidget::getStatusMsg() const
{
    return statusMessageLabel->text();
}

QString GenericChatroomWidget::getTitle() const
{
    QString title = getName();

    if (!getStatusString().isNull())
        title += QStringLiteral(" (") + getStatusString() + QStringLiteral(")");

    return title;
}

void GenericChatroomWidget::reloadTheme()
{
    QPalette p;

    p = statusMessageLabel->palette();
    p.setColor(QPalette::WindowText, Style::getColor(Style::LightGrey));       // Base color
    p.setColor(QPalette::HighlightedText, Style::getColor(Style::MediumGrey)); // Color when active
    statusMessageLabel->setPalette(p);

    p = nameLabel->palette();
    p.setColor(QPalette::WindowText, Style::getColor(Style::White));         // Base color
    p.setColor(QPalette::HighlightedText, Style::getColor(Style::DarkGrey)); // Color when active
    nameLabel->setPalette(p);

    p = palette();
    p.setColor(QPalette::Window, Style::getColor(Style::ThemeMedium));   // Base background color
    p.setColor(QPalette::Highlight, Style::getColor(Style::ThemeLight)); // On mouse over
    p.setColor(QPalette::Light, Style::getColor(Style::White));          // When active
    setPalette(p);
}

void GenericChatroomWidget::activate()
{
    emit chatroomWidgetClicked(this);
}

void GenericChatroomWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit chatroomWidgetClicked(this);
    else
        event->ignore();
}

void GenericChatroomWidget::enterEvent(QEvent*)
{
    if (!active)
        setBackgroundRole(QPalette::Highlight);
}

void GenericChatroomWidget::leaveEvent(QEvent* event)
{
    if (!active)
        setBackgroundRole(QPalette::Window);
    QWidget::leaveEvent(event);
}
