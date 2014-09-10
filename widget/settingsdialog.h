#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = 0);

public slots:
     void changePage(QListWidgetItem *current, QListWidgetItem *previous);

private:
    void createIcons();
    void readConfig();
    void writeConfig();

    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
};

#endif // CONFIGDIALOG_H
