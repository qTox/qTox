#include "callconfirmwidget.h"
#include "widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

CallConfirmWidget::CallConfirmWidget(QWidget *anchor) :
    QWidget(Widget::getInstance())
{
    Widget* w = Widget::getInstance();

    setWindowFlags(Qt::SubWindow);
    setAutoFillBackground(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *callLabel = new QLabel(tr("Incoming call..."), this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *accept = new QPushButton("Y", this), *reject = new QPushButton("N", this);

    buttonBox->addButton(accept, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(reject, QDialogButtonBox::RejectRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CallConfirmWidget::accepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CallConfirmWidget::rejected);

    layout->addWidget(callLabel);
    layout->addWidget(buttonBox);

    setFixedSize(150,90);
    move(anchor->mapToGlobal({(anchor->width()-width())/2,anchor->height()})-w->mapToGlobal({0,0}));
}
