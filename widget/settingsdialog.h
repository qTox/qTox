#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QDialog>

class Widget;
class SelfCamView;
class Camera;

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

    QLineEdit* userName;
    QLineEdit* statusMessage;
    QLineEdit* toxID;
};

class Privacy : public QWidget
{
public:
    Privacy(QWidget* parent = 0);
};

class AudioVideo : public QWidget
{
    Q_OBJECT
public:
    AudioVideo(QWidget* parent = 0);
    ~AudioVideo();

    QPushButton* testVideo;
    Camera* camera;
    SelfCamView* camView;

public slots:
    void onTestVideoPressed();
};



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
