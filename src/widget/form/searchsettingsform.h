#ifndef SEARCHSETTINGSFORM_H
#define SEARCHSETTINGSFORM_H

#include <QWidget>

namespace Ui {
class SearchSettingsForm;
}

class SearchSettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SearchSettingsForm(QWidget *parent = nullptr);
    ~SearchSettingsForm();

private:
    Ui::SearchSettingsForm *ui;

private slots:
    void onStartSearchSelected(const int index);
    void onRegisterClicked(const bool checked);
    void onWordsOnlyClicked(const bool checked);
    void onRegularClicked(const bool checked);
};

#endif // SEARCHSETTINGSFORM_H
