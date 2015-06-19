/*
    Copyright © 2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONTENTDIALOG_H
#define CONTENTDIALOG_H

#include <QDialog>
#include <tuple>

template <typename K, typename V> class QHash;

class QSplitter;
class QVBoxLayout;
class ContentLayout;
class GenericChatroomWidget;

class ContentDialog : public QDialog
{
    Q_OBJECT
public:
    ContentDialog(QWidget* parent = 0);
    ~ContentDialog();
    void addFriend(int friendId, QString id);
    static ContentDialog* current();
    static bool showChatroomWidget(int friendId);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onChatroomWidgetClicked(GenericChatroomWidget* widget);

private:
    void saveDialogGeometry();
    void saveSplitterState();

    QSplitter* splitter;
    QVBoxLayout* friendLayout;
    ContentLayout* contentLayout;
    GenericChatroomWidget* activeChatroomWidget;
    static ContentDialog* currentDialog;
    static QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> friendList;
};

#endif // CONTENTDIALOG_H
