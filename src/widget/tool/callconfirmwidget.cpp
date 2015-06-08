/*
    Copyright © 2015 by The qTox Project

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


#include "callconfirmwidget.h"
#include "src/widget/gui.h"
#include "src/widget/widget.h"
#include <assert.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QPalette>

CallConfirmWidget::CallConfirmWidget(const QWidget *Anchor, const Friend& f) :
    QWidget(GUI::getMainWidget()), anchor(Anchor), f(f),
    rectW{120}, rectH{85},
    spikeW{30}, spikeH{15},
    roundedFactor{20},
    rectRatio(static_cast<qreal>(rectH)/static_cast<qreal>(rectW))
{
    setWindowFlags(Qt::SubWindow);

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::white);
    setPalette(palette);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *callLabel = new QLabel(QObject::tr("Incoming call..."), this);
    callLabel->setWordWrap(true);
    callLabel->setAlignment(Qt::AlignHCenter);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *accept = new QPushButton(this), *reject = new QPushButton(this);
    accept->setFlat(true);
    reject->setFlat(true);
    accept->setStyleSheet("QPushButton{border:none;}");
    reject->setStyleSheet("QPushButton{border:none;}");
    accept->setIcon(QIcon(":/ui/acceptCall/acceptCall.svg"));
    reject->setIcon(QIcon(":/ui/rejectCall/rejectCall.svg"));
    accept->setIconSize(accept->size());
    reject->setIconSize(reject->size());

    buttonBox->addButton(accept, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(reject, QDialogButtonBox::RejectRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CallConfirmWidget::accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CallConfirmWidget::rejected);

    connect(&GUI::getInstance(), &GUI::resized, this, &CallConfirmWidget::reposition);

    layout->setMargin(12);
    layout->addSpacing(spikeH);
    layout->addWidget(callLabel);
    layout->addWidget(buttonBox);

    setFixedSize(rectW,rectH+spikeH);
    reposition();
}

void CallConfirmWidget::reposition()
{
    QWidget* w = GUI::getMainWidget();
    QPoint pos = anchor->mapToGlobal({(anchor->width()-rectW)/2,anchor->height()})-w->mapToGlobal({0,0});

    // We don't want the widget to overflow past the right of the screen
    int xOverflow=0;
    if (pos.x() + rectW > w->width())
        xOverflow = pos.x() + rectW - w->width();
    pos.rx() -= xOverflow;

    mainRect = {0,spikeH,rectW,rectH};
    brush = QBrush(QColor(65,65,65));
    spikePoly = QPolygon({{(rectW-spikeW)/2+xOverflow,spikeH},
                          {rectW/2+xOverflow,0},
                          {(rectW+spikeW)/2+xOverflow,spikeH}});

    move(pos);
    update();
}

void CallConfirmWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);

    painter.drawRoundRect(mainRect, roundedFactor*rectRatio, roundedFactor);
    painter.drawPolygon(spikePoly);
}

void CallConfirmWidget::showEvent(QShowEvent*)
{
    // If someone does show() from Widget or lower, the event will reach us
    // because it's our parent, and we could show up in the wrong form.
    // So check here if our friend's form is actually the active one.
    if (!Widget::getInstance()->isFriendWidgetCurActiveWidget(&f))
    {
        QWidget::hide();
        return;
    }
    reposition();
    update();
}
