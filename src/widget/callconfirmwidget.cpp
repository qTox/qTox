#include "callconfirmwidget.h"
#include "widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QPalette>

CallConfirmWidget::CallConfirmWidget(const QWidget *Anchor) :
    QWidget(Widget::getInstance()), anchor(Anchor),
    rectW{130}, rectH{90},
    spikeW{30}, spikeH{15},
    roundedFactor{15}
{
    setWindowFlags(Qt::SubWindow);

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::white);
    setPalette(palette);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *callLabel = new QLabel(tr("Incoming call..."), this);
    callLabel->setAlignment(Qt::AlignHCenter);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *accept = new QPushButton("Accept", this), *reject = new QPushButton("Reject", this);

    buttonBox->addButton(accept, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(reject, QDialogButtonBox::RejectRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CallConfirmWidget::accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CallConfirmWidget::rejected);

    connect(Widget::getInstance(), &Widget::resized, this, &CallConfirmWidget::reposition);

    layout->setMargin(12);
    layout->addSpacing(spikeH);
    layout->addWidget(callLabel);
    layout->addWidget(buttonBox);

    setFixedSize(rectW,rectH+spikeH);
    reposition();
}

void CallConfirmWidget::reposition()
{
    Widget* w = Widget::getInstance();
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

    painter.drawRoundRect(mainRect, roundedFactor, roundedFactor);
    painter.drawPolygon(spikePoly);
}

void CallConfirmWidget::showEvent(QShowEvent*)
{
    reposition();
    update();
}
