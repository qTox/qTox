/*
    Copyright © 2016 by The qTox Project

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

#include "contentwidget.h"

#include "src/widget/style.h"

#include <QFrame>
#include <QPalette>
#include <QVBoxLayout>

/**
 * @class   ContentWidget
 * @brief   Provides a widget with a fixed 2-row (header, body) layout.
 *
 * The widget defines a header widget and a body widget, which can be used
 * or replaced by the inheriting class. Header and body are separated by a thin
 * horizontal line to visually separate them.
 */

/**
 * @brief       ContentWidget constructor
 * @param[in]   parent  the parent widget (may be nullptr)
 * @param[in]   f       optional window flags; defaults to Qt::Window
 */
ContentWidget::ContentWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f | Qt::Window)
    , contentLayout(new QVBoxLayout(this))
{
    setAttribute(Qt::WA_DeleteOnClose);

    setContentsMargins(0,0,0,0);
    contentLayout->setMargin(0);

    // TODO: the palette has to be set by the style implementation instead
    //       of using a fixed one.
    QPalette pal = palette();

    pal.setBrush(QPalette::WindowText, QColor(0, 0, 0));
    pal.setBrush(QPalette::Button, QColor(255, 255, 255));
    pal.setBrush(QPalette::Light, QColor(255, 255, 255));
    pal.setBrush(QPalette::Midlight, QColor(255, 255, 255));
    pal.setBrush(QPalette::Dark, QColor(127, 127, 127));
    pal.setBrush(QPalette::Mid, QColor(170, 170, 170));
    pal.setBrush(QPalette::Text, QColor(0, 0, 0));
    pal.setBrush(QPalette::BrightText, QColor(255, 255, 255));
    pal.setBrush(QPalette::ButtonText, QColor(0, 0, 0));
    pal.setBrush(QPalette::Base, QColor(255, 255, 255));
    pal.setBrush(QPalette::Window, QColor(255, 255, 255));
    pal.setBrush(QPalette::Shadow, QColor(0, 0, 0));
    pal.setBrush(QPalette::AlternateBase, QColor(255, 255, 255));
    pal.setBrush(QPalette::ToolTipBase, QColor(255, 255, 220));
    pal.setBrush(QPalette::ToolTipText, QColor(0, 0, 0));

    pal.setBrush(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    pal.setBrush(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    pal.setBrush(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));

    setPalette(pal);

    // horizontal line
    mainHLine.setFrameShape(QFrame::HLine);
    mainHLine.setFrameShadow(QFrame::Plain);
    mainHLine.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    // TODO: the palette has to be set by the style implementation instead
    //       of using a fixed one.
    QPalette palette = mainHLine.palette();
    palette.setBrush(QPalette::WindowText, QBrush(QColor(193, 193, 193)));
    mainHLine.setPalette(palette);
}

void ContentWidget::setupLayout(QWidget* head, QWidget* body)
{
    QLayoutItem* item = nullptr;
    while ((item = contentLayout->takeAt(0)) != nullptr)
    {
        delete item;
    }

    head->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    contentLayout->addWidget(head);
    contentLayout->addWidget(&mainHLine);
    contentLayout->addSpacing(5);
    contentLayout->addWidget(body);
}

QWidget* ContentWidget::headerWidget() const
{
    QLayoutItem* item = contentLayout->itemAt(0);
    return item ? item->widget() : nullptr;
}

QWidget* ContentWidget::bodyWidget() const
{
    QLayoutItem* item = contentLayout->itemAt(3);
    return item ? item->widget() : nullptr;
}

QSize ContentWidget::minimumSizeHint() const
{
    return QSize(640, 480);
}
