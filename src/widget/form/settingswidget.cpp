/*
    Copyright © 2014-2018 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "settingswidget.h"

#include "src/audio/audio.h"
#include "src/core/coreav.h"
#include "src/core/core.h"
#include "src/persistence/settings.h"
#include "src/video/camerasource.h"
#include "src/widget/contentlayout.h"
#include "src/widget/form/settings/aboutform.h"
#include "src/widget/form/settings/advancedform.h"
#include "src/widget/form/settings/avform.h"
#include "src/widget/form/settings/generalform.h"
#include "src/widget/form/settings/privacyform.h"
#include "src/widget/form/settings/userinterfaceform.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

#include <QLabel>
#include <QTabWidget>
#include <QWindow>

#include <memory>

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    Audio* audio = &Audio::getInstance();
    CoreAV* coreAV = Core::getInstance()->getAv();
    IAudioSettings* audioSettings = &Settings::getInstance();
    IVideoSettings* videoSettings = &Settings::getInstance();
    CameraSource& camera = CameraSource::getInstance();

    setAttribute(Qt::WA_DeleteOnClose);

    bodyLayout = std::unique_ptr<QVBoxLayout>(new QVBoxLayout());

    settingsWidgets = std::unique_ptr<QTabWidget>(new QTabWidget(this));
    settingsWidgets->setTabPosition(QTabWidget::North);
    bodyLayout->addWidget(settingsWidgets.get());

    std::unique_ptr<GeneralForm> gfrm(new GeneralForm(this));
    std::unique_ptr<UserInterfaceForm> uifrm(new UserInterfaceForm(this));
    std::unique_ptr<PrivacyForm> pfrm(new PrivacyForm());
    AVForm* rawAvfrm = new AVForm(audio, coreAV, camera, audioSettings, videoSettings);
    std::unique_ptr<AVForm> avfrm(rawAvfrm);
    std::unique_ptr<AdvancedForm> expfrm(new AdvancedForm());
    std::unique_ptr<AboutForm> abtfrm(new AboutForm());

    cfgForms = {{std::move(gfrm), std::move(uifrm), std::move(pfrm), std::move(avfrm), std::move(expfrm), std::move(abtfrm)}};
    for (auto& cfgForm : cfgForms)
        settingsWidgets->addTab(cfgForm.get(), cfgForm->getFormIcon(), cfgForm->getFormName());

    connect(settingsWidgets.get(), &QTabWidget::currentChanged, this, &SettingsWidget::onTabChanged);

    Translator::registerHandler(std::bind(&SettingsWidget::retranslateUi, this), this);
}

SettingsWidget::~SettingsWidget()
{
    Translator::unregister(this);
}

void SettingsWidget::setBodyHeadStyle(QString style)
{
    settingsWidgets->setStyle(QStyleFactory::create(style));
}

void SettingsWidget::showAbout()
{
    onTabChanged(settingsWidgets->count() - 1);
}

bool SettingsWidget::isShown() const
{
    if (settingsWidgets->isVisible()) {
        settingsWidgets->window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void SettingsWidget::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(settingsWidgets.get());
    settingsWidgets->show();
    onTabChanged(settingsWidgets->currentIndex());
}

void SettingsWidget::onTabChanged(int index)
{
    settingsWidgets->setCurrentIndex(index);
}

void SettingsWidget::retranslateUi()
{
    for (size_t i = 0; i < cfgForms.size(); ++i)
        settingsWidgets->setTabText(i, cfgForms[i]->getFormName());
}
