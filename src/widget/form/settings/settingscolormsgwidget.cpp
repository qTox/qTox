#include "settingscolormsgwidget.h"
#include <QColorDialog>
#include <QHBoxLayout>
#include <QSpacerItem>

SettingsColorMsgWidget::SettingsColorMsgWidget(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);

    chooseColorButton = new QPushButton(tr("Choose Color"));
    boldCheckBox = new QCheckBox(tr("bold"));
    italicCheckBox = new QCheckBox(tr("italic"));

    layout->addWidget(chooseColorButton);
    layout->addWidget(boldCheckBox);
    layout->addWidget(italicCheckBox);
    layout->addSpacerItem(new QSpacerItem(0,10, QSizePolicy::Expanding, QSizePolicy::Expanding));

    setLayout(layout);

    connect(chooseColorButton, &QPushButton::clicked, this, &SettingsColorMsgWidget::chooseColor);
    connect(boldCheckBox, &QCheckBox::stateChanged, this, &SettingsColorMsgWidget::checkBold);
    connect(italicCheckBox, &QCheckBox::stateChanged, this, &SettingsColorMsgWidget::checkItalic);
}

void SettingsColorMsgWidget::setColor(const QString& c)
{
    color = QColor(c);
}

void SettingsColorMsgWidget::setCheckedBold(bool state)
{
    boldCheckBox->setChecked(state);
}

void SettingsColorMsgWidget::setCheckedItalic(bool state)
{
    italicCheckBox->setChecked(state);
}

void SettingsColorMsgWidget::chooseColor()
{
    QColorDialog cd(color);
    cd.exec();

    color = cd.currentColor();
    emit selectedColor(color.name());
}
