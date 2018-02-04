#ifndef SEARCHFORM_H
#define SEARCHFORM_H

#include <QWidget>

class QPushButton;
class QLineEdit;

class SearchForm final : public QWidget
{
    Q_OBJECT
public:
    explicit SearchForm(QWidget *parent = nullptr);

private:
    QPushButton* upButton;
    QPushButton* downButton;
    QLineEdit* searchLine;
};

#endif // SEARCHFORM_H
