#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QWidget>
#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;


// =======================================
// settings pages
//========================================
class GeneralPage : public QWidget
{
public:
    GeneralPage(QWidget *parent = 0);
};

class Identity : public QWidget
{
public:
    Identity(QWidget* parent = 0);
};

class Privacy : public QWidget
{
public:
    Privacy(QWidget* parent = 0);
};



// =======================================
// settings dialog
//========================================
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
