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

/** TODO
 *
 * Sort the friend list by status, online first then busy then offline
 * Don't do anything if a friend is disconnected, don't print to the chat
 *
 */
