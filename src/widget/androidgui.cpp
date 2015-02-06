#include "androidgui.h"
#include <QLabel>

AndroidGUI::AndroidGUI(QWidget *parent) :
    QWidget(parent)
{
    l = new QLabel("qTox Android", this);
}

AndroidGUI::~AndroidGUI()
{
    delete l;
}
