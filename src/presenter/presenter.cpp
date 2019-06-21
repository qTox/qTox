//
// Created by kriby on 6/17/19.
//

#include "presenter.h"
#include "src/core/coreav.h"
#include "src/model/groupinvite.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/video/videoframe.h"
#include "src/widget/gui.h"
#include "src/widget/loginscreen.h"
#include "src/widget/widget.h"
#include "tox/toxav.h"
#include "vpx/vpx_image.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QtCore/QSignalMapper>


Q_DECLARE_OPAQUE_POINTER(ToxAV*)

Presenter::Presenter() = default;

/**
 * @brief Sets up invariants
 */
void Presenter::setupEnvironment()
{
    qDebug() << "Starting up";

    // Setup the environment
    qRegisterMetaType<Status::Status>("Status::Status");
    qRegisterMetaType<vpx_image>("vpx_image");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<const int16_t*>("const int16_t*");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<QPixmap>("QPixmap");
    qRegisterMetaType<Profile*>("Profile*");
    qRegisterMetaType<ToxAV*>("ToxAV*");
    qRegisterMetaType<ToxFile>("ToxFile");
    qRegisterMetaType<ToxFile::FileDirection>("ToxFile::FileDirection");
    qRegisterMetaType<std::shared_ptr<VideoFrame>>("std::shared_ptr<VideoFrame>");
    qRegisterMetaType<ToxPk>("ToxPk");
    qRegisterMetaType<ToxId>("ToxId");
    qRegisterMetaType<ToxPk>("GroupId");
    qRegisterMetaType<ToxPk>("ContactId");
    qRegisterMetaType<GroupInvite>("GroupInvite");
    qRegisterMetaType<ReceiptNum>("ReceiptNum");
    qRegisterMetaType<RowId>("RowId");

    qApp->setQuitOnLastWindowClosed(false);
}

#ifdef Q_OS_MAC
// TODO: still needed?
void Presenter::setupMacEnvironment()
{
    globalMenuBar = new QMenuBar(0);
    dockMenu = new QMenu(globalMenuBar);

    viewMenu = globalMenuBar->addMenu(QString());

    windowMenu = globalMenuBar->addMenu(QString());
    globalMenuBar->addAction(windowMenu->menuAction());

    fullscreenAction = viewMenu->addAction(QString());
    fullscreenAction->setShortcut(QKeySequence::FullScreen);
    connect(fullscreenAction, &QAction::triggered, this, &Presenter::toggleFullscreen);

    minimizeAction = windowMenu->addAction(QString());
    minimizeAction->setShortcut(Qt::CTRL + Qt::Key_M);
    connect(minimizeAction, &QAction::triggered, [this]() {
        minimizeAction->setEnabled(false);
        QApplication::focusWindow()->showMinimized();
    });

    windowMenu->addSeparator();
    frontAction = windowMenu->addAction(QString());
    connect(frontAction, &QAction::triggered, this, &Presenter::bringAllToFront);

    QAction* quitAction = new QAction(globalMenuBar);
    quitAction->setMenuRole(QAction::QuitRole);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QObject*)), this, SLOT(onOpenWindow(QObject*)));

    retranslateUi();
}
void Presenter::retranslateUi()
{
    viewMenu->menuAction()->setText(tr("View", "OS X Menu bar"));
    windowMenu->menuAction()->setText(tr("Window", "OS X Menu bar"));
    minimizeAction->setText(tr("Minimize", "OS X Menu bar"));
    frontAction->setText((tr("Bring All to Front", "OS X Menu bar")));
}

void Presenter::onWindowStateChanged(Qt::WindowStates state)
{
    minimizeAction->setEnabled(QApplication::activeWindow() != nullptr);

    if (QApplication::activeWindow() != nullptr && sender() == QApplication::activeWindow()) {
        if (state & Qt::WindowFullScreen)
            minimizeAction->setEnabled(false);

        if (state & Qt::WindowFullScreen)
            fullscreenAction->setText(tr("Exit Fullscreen"));
        else
            fullscreenAction->setText(tr("Enter Fullscreen"));

        updateWindows();
    }

    updateWindowsStates();
}

void Presenter::updateWindows()
{
    updateWindowsArg(nullptr);
}

void Presenter::updateWindowsArg(QWindow* closedWindow)
{
    QWindowList windowList = QApplication::topLevelWindows();
    delete windowActions;
    windowActions = new QActionGroup(this);

    windowMenu->addSeparator();

    QAction* dockLast;
    if (!dockMenu->actions().isEmpty())
        dockLast = dockMenu->actions().first();
    else
        dockLast = nullptr;

    QWindow* activeWindow;

    if (QApplication::activeWindow())
        activeWindow = QApplication::activeWindow()->windowHandle();
    else
        activeWindow = nullptr;

    for (int i = 0; i < windowList.size(); ++i) {
        if (closedWindow == windowList[i])
            continue;

        QAction* action = windowActions->addAction(windowList[i]->title());
        action->setCheckable(true);
        action->setChecked(windowList[i] == activeWindow);
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, windowList[i]);
        windowMenu->addAction(action);
        dockMenu->insertAction(dockLast, action);
    }

    if (dockLast && !dockLast->isSeparator())
        dockMenu->insertSeparator(dockLast);
}

void Presenter::updateWindowsClosed()
{
    updateWindowsArg(static_cast<QWidget*>(sender())->windowHandle());
}

void Presenter::updateWindowsStates()
{
    bool exists = false;
    QWindowList windowList = QApplication::topLevelWindows();

    for (QWindow* window : windowList) {
        if (!(window->windowState() & Qt::WindowMinimized)) {
            exists = true;
            break;
        }
    }

    frontAction->setEnabled(exists);
}

void Presenter::onOpenWindow(QObject* object)
{
    QWindow* window = static_cast<QWindow*>(object);

    if (window->windowState() & QWindow::Minimized)
        window->showNormal();

    window->raise();
    window->requestActivate();
}

void Presenter::toggleFullscreen()
{
    QWidget* window = QApplication::activeWindow();

    if (window->isFullScreen())
        window->showNormal();
    else
        window->showFullScreen();
}

void Presenter::bringAllToFront()
{
    QWindowList windowList = QApplication::topLevelWindows();
    QWindow* focused = QApplication::focusWindow();

    for (QWindow* window : windowList)
        window->raise();

    focused->raise();
}
#endif