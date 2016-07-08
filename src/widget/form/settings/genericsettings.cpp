#include "genericsettings.h"

#include <QEvent>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>

GenericForm::GenericForm(const QPixmap &icon)
    : formIcon(icon)
{
}

QPixmap GenericForm::getFormIcon()
{
    return formIcon;
}

void GenericForm::eventsInit()
{
    // prevent stealing mouse wheel scroll
    // scrolling event won't be transmitted to comboboxes or qspinboxes when scrolling
    // you can scroll through general settings without accidentially changing theme/skin/icons etc.
    // @see GenericForm::eventFilter(QObject *o, QEvent *e) at the bottom of this file for more
    for (QComboBox *cb : findChildren<QComboBox*>())
    {
        cb->installEventFilter(this);
        cb->setFocusPolicy(Qt::StrongFocus);
    }

    for (QSpinBox *sp : findChildren<QSpinBox*>())
    {
        sp->installEventFilter(this);
        sp->setFocusPolicy(Qt::WheelFocus);
    }

    for (QCheckBox *cb : findChildren<QCheckBox*>()) // this one is to allow scrolling on checkboxes
        cb->installEventFilter(this);
}

bool GenericForm::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Wheel) &&
         (qobject_cast<QComboBox*>(o) ||
          qobject_cast<QAbstractSpinBox*>(o) ||
          qobject_cast<QCheckBox*>(o)))
    {
        e->ignore();
        return true;
    }
    return QWidget::eventFilter(o, e);
}
