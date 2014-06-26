#include "widget/widget.h"
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
 * ">using a dedicated tool to maintain a TODO list" edition
 *
 * Audio boilerplate, able to create/accept/reject/hang empty audio calls
 * Play received sound during audio calls
 * Record sound during audio calls, and send it
 * Sort the friend list by status, online first then busy then offline
 * Don't do anything if a friend is disconnected, don't print to the chat
 * Show the picture's size between name and size after transfer completion if it's a pic
 * Adjust all status icons to match the mockup, including scooting the friendslist ones to the left and making the user one the same size
 * Sidepanel (friendlist) should be resizeable
 * The online/offline/away status at the top (our) is way too big i think (follow the mockup/uTox)
 * Color change of friendlist widgets on hover/selected
 * In-chat messages should have line wrapping
 * An extra side panel for groupchats, like Venom does (?)
 *
 */
