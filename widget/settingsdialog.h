#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QDialog>

class Widget;
class SelfCamView;
class Camera;
class General;
class Identity;
class Privacy;
class AudioVideo;

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QPushButton;
class QCheckBox;
class QLineEdit;

// =======================================
// settings dialog
//========================================
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(Widget *parent);

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

    Widget* widget;

    // pages
    General*    generalPage;
    Identity*   identityPage;
    Privacy*    privacyPage;
    AudioVideo* audioVideoPage;
    QListWidget*    contentsWidget;
    QStackedWidget* pagesWidget;

    // buttons
    QPushButton* okButton;
    QPushButton* cancelButton;
    QPushButton* applyButton;
};

#endif // SETTINGSDIALOG_H
