#include "settingsdialog.h"
#include "settings.h"
#include "widget.h"
#include "camera.h"
#include "selfcamview.h"
#include "core.h"
#include "smileypack.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>


// =======================================
// settings pages
//========================================
class GeneralPage : public QWidget
{
public:
    GeneralPage(QWidget *parent = 0) :
        QWidget(parent)
    {
        QGroupBox *group = new QGroupBox(tr("General Settings"), this);

        enableIPv6 = new QCheckBox(this);
        enableIPv6->setText(tr("Enable IPv6 (recommended)","Text on a checkbox to enable IPv6"));
        useTranslations = new QCheckBox(this);
        useTranslations->setText(tr("Use translations","Text on a checkbox to enable translations"));
        makeToxPortable = new QCheckBox(this);
        makeToxPortable->setText(tr("Make Tox portable","Text on a checkbox to make qTox a portable application"));
        makeToxPortable->setToolTip(tr("Save settings to the working directory instead of the usual conf dir","describes makeToxPortable checkbox"));

        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addWidget(enableIPv6);
        vLayout->addWidget(useTranslations);
        vLayout->addWidget(makeToxPortable);
        group->setLayout(vLayout);

        // theme
        QGroupBox* themeGroup = new QGroupBox(tr("Theme"));
        QLabel* smileyLabel = new QLabel(tr("Smiley Pack"));
        smileyPack = new QComboBox(this);

        auto smileyPacks = SmileyPack::listSmileyPacks();
        for(auto pack : smileyPacks)
            smileyPack->addItem(QString("%1 (%2)").arg(pack.first).arg(pack.second), pack.second);

        QVBoxLayout* themeLayout = new QVBoxLayout();
        themeLayout->addWidget(smileyLabel);
        themeLayout->addWidget(smileyPack);
        themeGroup->setLayout(themeLayout);

        QVBoxLayout *mainLayout = new QVBoxLayout();
        mainLayout->addWidget(group);
        mainLayout->addWidget(themeGroup);
        mainLayout->addStretch(1);
        setLayout(mainLayout);
    }

    QCheckBox* enableIPv6;
    QCheckBox* useTranslations;
    QCheckBox* makeToxPortable;
    QComboBox* smileyPack;
};

class IdentityPage : public QWidget
{
public:
    IdentityPage(QWidget* parent = 0) :
        QWidget(parent)
    {
        // public
        QGroupBox *publicGroup = new QGroupBox(tr("Public Information"), this);
        QLabel* userNameLabel = new QLabel(tr("Name","Username/nick"), this);
        userName = new QLineEdit(this);
        QLabel* statusMessageLabel = new QLabel(tr("Status","Status message"), this);
        statusMessage = new QLineEdit(this);
        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addWidget(userNameLabel);
        vLayout->addWidget(userName);
        vLayout->addWidget(statusMessageLabel);
        vLayout->addWidget(statusMessage);
        publicGroup->setLayout(vLayout);

        // tox
        QGroupBox* toxGroup = new QGroupBox(tr("Tox ID"), this);
        QLabel* toxIdLabel = new QLabel(tr("Your Tox ID"), this);
        toxID = new QLineEdit(this);
        toxID->setReadOnly(true);
        QVBoxLayout* toxLayout = new QVBoxLayout();
        toxLayout->addWidget(toxIdLabel);
        toxLayout->addWidget(toxID);
        toxGroup->setLayout(toxLayout);

        QVBoxLayout *mainLayout = new QVBoxLayout();
        mainLayout->setSpacing(30);
        mainLayout->addWidget(publicGroup);
        mainLayout->addWidget(toxGroup);
        mainLayout->addStretch(1);
        setLayout(mainLayout);
    }

    QLineEdit* userName;
    QLineEdit* statusMessage;
    QLineEdit* toxID;
};

class PrivacyPage : public QWidget
{
public:
    PrivacyPage(QWidget* parent = 0) :
        QWidget(parent)
    {}
};

class AVPage : public QWidget
{
    Q_OBJECT
public:
    AVPage(SettingsDialog* parent = 0) :
        QWidget(parent)
    {
        QGroupBox *group = new QGroupBox(tr("Video Settings"), this);

        camView = new SelfCamView(parent->getWidget()->getCamera());
        camView->hide(); // hide by default
        testVideo = new QPushButton("Show video preview");
        connect(testVideo, SIGNAL(clicked()), this, SLOT(onTestVideoPressed()));

        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addWidget(testVideo);
        vLayout->addWidget(camView);
        group->setLayout(vLayout);

        QVBoxLayout *mainLayout = new QVBoxLayout();
        mainLayout->addWidget(group);
        mainLayout->addStretch(1);
        setLayout(mainLayout);
    }

    ~AVPage()
    {
        delete camView;
    }

    void showTestVideo()
    {
        testVideo->setText("Hide video preview");
        camView->show();
    }

    void closeTestVideo()
    {
        testVideo->setText("Show video preview");
        camView->close();
    }

    QPushButton* testVideo;
    SelfCamView* camView;

public slots:
    void onTestVideoPressed()
    {
        if (camView->isVisible()) {
            closeTestVideo();
        } else {
            showTestVideo();
        }
    }
};

// allows Q_OBJECT macro inside cpp
#include "settingsdialog.moc"



// =======================================
// settings dialog
//========================================
SettingsDialog::SettingsDialog(Widget *parent) :
    QDialog(parent),
    widget(parent)
{
    createPages();
    createButtons();
    createConnections();
    createLayout();
    setWindowTitle(tr("qTox – Settings"));
}

void SettingsDialog::createPages()
{
    generalPage  = new GeneralPage(this);
    identityPage = new IdentityPage(this);
    privacyPage  = new PrivacyPage(this);
    avPage       = new AVPage(this);

    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(100, 73));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(110);
    contentsWidget->setMinimumWidth(110);
    contentsWidget->setSpacing(0);
    contentsWidget->setFlow(QListView::TopToBottom);

    pagesWidget = new QStackedWidget;
    pagesWidget->addWidget(generalPage);
    pagesWidget->addWidget(identityPage);
    pagesWidget->addWidget(privacyPage);
    pagesWidget->addWidget(avPage);

    QListWidgetItem *generalButton = new QListWidgetItem(contentsWidget);
    generalButton->setIcon(QIcon(":/img/settings/general.png"));
    generalButton->setText(tr("General"));
    generalButton->setTextAlignment(Qt::AlignHCenter);
    generalButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *identity = new QListWidgetItem(contentsWidget);
    identity->setIcon(QIcon(":/img/settings/identity.png"));
    identity->setText(tr("Identity"));
    identity->setTextAlignment(Qt::AlignHCenter);
    identity->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *privacy = new QListWidgetItem(contentsWidget);
    privacy->setIcon(QIcon(":/img/settings/privacy.png"));
    privacy->setText(tr("Privacy"));
    privacy->setTextAlignment(Qt::AlignHCenter);
    privacy->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *av = new QListWidgetItem(contentsWidget);
    av->setIcon(QIcon(":/img/settings/av.png"));
    av->setText(tr("Audio/Video"));
    av->setTextAlignment(Qt::AlignHCenter);
    av->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    contentsWidget->setCurrentRow(0);
}

void SettingsDialog::createButtons()
{
    okButton     = new QPushButton(tr("Ok"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);
    applyButton  = new QPushButton(tr("Apply"), this);
}

void SettingsDialog::createConnections()
{
    connect(okButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(applyPressed()));
    connect(
        contentsWidget,
        SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
        this,
        SLOT(changePage(QListWidgetItem*,QListWidgetItem*))
    );
}

void SettingsDialog::createLayout()
{
    setMinimumSize(800, 500);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(applyButton);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(contentsWidget);
    hLayout->addWidget(pagesWidget, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(hLayout);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
}

void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current) {
        current = previous;
    }
    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void SettingsDialog::okPressed()
{
    writeConfig();
    close();
}

void SettingsDialog::cancelPressed()
{
    close();
}

void SettingsDialog::applyPressed()
{
    writeConfig();
}

void SettingsDialog::readConfig()
{
    Settings& settings = Settings::getInstance();
    Core* core = widget->getCore();

    generalPage->enableIPv6->setChecked(settings.getEnableIPv6());
    generalPage->useTranslations->setChecked(settings.getUseTranslations());
    generalPage->makeToxPortable->setChecked(settings.getMakeToxPortable());

    identityPage->userName->setText(core->getUsername());
    identityPage->statusMessage->setText(core->getStatusMessage());
    identityPage->toxID->setText(core->getSelfId().toString());
}

void SettingsDialog::writeConfig()
{
    Settings& settings = Settings::getInstance();
    Core* core = widget->getCore();


    // only save settings if something changed
    bool saveSettings = false;
    if (settings.getEnableIPv6() != generalPage->enableIPv6->isChecked()) {
        settings.setEnableIPv6(generalPage->enableIPv6->isChecked());
        saveSettings = true;
    }

    if (settings.getUseTranslations() != generalPage->useTranslations->isChecked()) {
        settings.setUseTranslations(generalPage->useTranslations->isChecked());
        saveSettings = true;
    }

    if (settings.getMakeToxPortable() != generalPage->makeToxPortable->isChecked()) {
        settings.setMakeToxPortable(generalPage->makeToxPortable->isChecked());
        saveSettings = true;
    }

    if (settings.getSmileyPack() != generalPage->smileyPack->currentData().toString()) {
        settings.setSmileyPack(generalPage->smileyPack->currentData().toString());
        saveSettings = true;
    }

    if (saveSettings) {
        settings.save();
    }


    // changing core settings will automatically save them
    QString userName = identityPage->userName->text();
    if (core->getUsername() != userName) {
        core->setUsername(userName);
    }

    QString statusMessage = identityPage->statusMessage->text();
    if (core->getStatusMessage() != statusMessage) {
        core->setStatusMessage(statusMessage);
    }
}

Widget* SettingsDialog::getWidget()
{
    return widget;
}

void SettingsDialog::closeEvent(QCloseEvent* e){
    avPage->closeTestVideo();
    QDialog::closeEvent(e);
}
