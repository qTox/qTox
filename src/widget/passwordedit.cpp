#include "passwordedit.h"
#ifdef QTOX_PLATFORM_EXT
#include "src/platform/capslock.h"
#endif
#include <QCoreApplication>

// It isn't needed for OSX, because it shows indicator by default
#if defined(QTOX_PLATFORM_EXT) && !defined(Q_OS_OSX)
#define ENABLE_CAPSLOCK_INDICATOR
#endif

PasswordEdit::EventHandler* PasswordEdit::eventHandler{nullptr};

PasswordEdit::PasswordEdit(QWidget* parent) :
    QLineEdit(parent),
    action(new QAction(this))
{
    setEchoMode(QLineEdit::Password);

#ifndef ENABLE_CAPSLOCK_INDICATOR
    action->setVisible(false);
#else
    action->setIcon(QIcon(":img/caps_lock.svg"));
    action->setToolTip(tr("CAPS-LOCK ENABLED"));
#endif

    addAction(action, QLineEdit::TrailingPosition);
}

PasswordEdit::~PasswordEdit()
{
    unregisterHandler();
}

void PasswordEdit::registerHandler()
{
#ifdef ENABLE_CAPSLOCK_INDICATOR
    if (!eventHandler)
        eventHandler = new EventHandler();
    if (!eventHandler->actions.contains(action))
        eventHandler->actions.append(action);
#endif
}

void PasswordEdit::unregisterHandler()
{
#ifdef ENABLE_CAPSLOCK_INDICATOR
    if (eventHandler && eventHandler->actions.contains(action))
    {
        eventHandler->actions.removeOne(action);
        if (eventHandler->actions.isEmpty())
        {
            delete eventHandler;
            eventHandler = nullptr;
        }
    }
#endif
}

void PasswordEdit::showEvent(QShowEvent*)
{
#ifdef ENABLE_CAPSLOCK_INDICATOR
    action->setVisible(Platform::capsLockEnabled());
#endif
    registerHandler();
}

void PasswordEdit::hideEvent(QHideEvent*)
{
    unregisterHandler();
}

#ifdef ENABLE_CAPSLOCK_INDICATOR
PasswordEdit::EventHandler::EventHandler()
{
    QCoreApplication::instance()->installEventFilter(this);
}

PasswordEdit::EventHandler::~EventHandler()
{
    QCoreApplication::instance()->removeEventFilter(this);
}

void PasswordEdit::EventHandler::updateActions()
{
    bool caps = Platform::capsLockEnabled();

    for (QAction* action : actions)
        action->setVisible(caps);
}

bool PasswordEdit::EventHandler::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::WindowActivate:
    case QEvent::KeyRelease:
        updateActions();
        break;
    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}
#endif // ENABLE_CAPSLOCK_INDICATOR
