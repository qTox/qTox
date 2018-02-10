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
    void removeText();

private:
    QPushButton* upButton;
    QPushButton* downButton;
    QLineEdit* searchLine;

    QString searchPhrase;

private slots:
    void changedSearchPhrare(const QString &text);
    void clickedUp();
    void clickedDown();

signals:
    void searchInBegin(const QString &);
    void searchUp(const QString &);
    void searchDown(const QString &);
};

#endif // SEARCHFORM_H
