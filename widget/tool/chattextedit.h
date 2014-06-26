#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>

class ChatTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget *parent = 0);
    virtual void keyPressEvent(QKeyEvent * event) override;

signals:
    void enterPressed();

public slots:

};

#endif // CHATTEXTEDIT_H
