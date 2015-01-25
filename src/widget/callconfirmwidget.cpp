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

CallConfirmWidget::CallConfirmWidget(QWidget *anchor) :
    QWidget(Widget::getInstance()),
    rectW{130}, rectH{90},
    spikeW{30}, spikeH{15},
    roundedFactor{15}
{
    Widget* w = Widget::getInstance();

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

    layout->setMargin(12);
    layout->addSpacing(spikeH);
    layout->addWidget(callLabel);
    layout->addWidget(buttonBox);

    mainRect = {0,spikeH,rectW,rectH};
    brush = QBrush(QColor(65,65,65));
    spikePoly = QPolygon({{(rectW-spikeW)/2,spikeH},{rectW/2,0},{(rectW+spikeW)/2,spikeH}});

    setFixedSize(rectW,rectH+spikeH);
    move(anchor->mapToGlobal({(anchor->width()-width())/2,anchor->height()})-w->mapToGlobal({0,0}));
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
