#include "chattextedit.h"
#include <QKeyEvent>

ChatTextEdit::ChatTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
}

void ChatTextEdit::keyPressEvent(QKeyEvent * event)
{
    int key = event->key();
    if ((key == Qt::Key_Enter || key == Qt::Key_Return)
            && !(event->modifiers() && Qt::ShiftModifier))
    {
        emit enterPressed();
        return;
    }
    QTextEdit::keyPressEvent(event);
}
