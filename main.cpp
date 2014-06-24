#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Toxgui");
    a.setOrganizationName("Tox");
    Widget* w = Widget::getInstance();
    w->show();

    int errorcode = a.exec();

    delete w;

    return errorcode;
}
