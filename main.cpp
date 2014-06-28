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
 * Sending large files (~380MB) "restarts" after ~10MB. Goes back to 0%, consumes twice as much ram (reloads the file?)
 * => Don't load the whole file at once, load small chunks (25MB?) when needed, then free them and load the next
 * Notifications/ringing when a call is received
 * Sort the friend list by status, online first then busy then offline
 * Don't do anything if a friend is disconnected, don't print to the chat
 * Show the picture's size between name and size after transfer completion if it's a pic
 * Adjust all status icons to match the mockup, including scooting the friendslist ones to the left and making the user one the same size
 * Sidepanel (friendlist) should be resizeable
 * The online/offline/away status at the top (our) is way too big i think (follow the mockup/uTox)
 * An extra side panel for groupchats, like Venom does (?)
 *
 * In the file transfer widget:
 * >There is more padding on the left side compared to the right.
 * >Maybe put the file size should be in the same row as the name.
 * >Right-align the ETA.
 *
 */

/** NAMES :
Botox
Ricin
Anthrax
Sarin
Cyanide
Polonium
Mercury
Arsenic
qTox
plague
Britney
Nightshade
Belladonna
toxer
GoyIM
 */
