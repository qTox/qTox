#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QWidget>
#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QPushButton;
class QCheckBox;
class QLineEdit;


// =======================================
// settings pages
//========================================
class General : public QWidget
{
public:
    General(QWidget *parent = 0);

    QCheckBox* enableIPv6;
    QCheckBox* useTranslations;
    QCheckBox* makeToxPortable;
};

class Identity : public QWidget
{
public:
    Identity(QWidget* parent = 0);

    QLineEdit* name;
    QLineEdit* status;
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

    void readConfig();
    void writeConfig();

public slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void okPressed();
    void cancelPressed();
    void applyPressed();

private:
    void createPages();
    void createButtons();
    void createConnections();

    // pages
    General*  generalPage;
    Identity* identityPage;
    Privacy*  privacyPage;
    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;

    // buttons
    QPushButton* okButton;
    QPushButton* cancelButton;
    QPushButton* applyButton;
};

#endif // CONFIGDIALOG_H
