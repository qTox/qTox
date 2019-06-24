/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>

class ChatTextEdit final : public QTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget* parent = nullptr);
    ~ChatTextEdit();
    void setLastMessage(QString lm);
    void sendKeyEvent(QKeyEvent* event);

signals:
    void enterPressed();
    void tabPressed();
    void keyPressed();
    void pasteImage(const QPixmap& pixmap);

protected:
    virtual void keyPressEvent(QKeyEvent* event) final override;

private:
    void retranslateUi();
    bool pasteIfImage(QKeyEvent* event);

private:
    QString lastMessage;
};

#endif // CHATTEXTEDIT_H
