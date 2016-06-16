#include "capslockindicator.h"
#ifdef QTOX_PLATFORM_EXT
#include "src/platform/capslock.h"
#endif
#include <QCoreApplication>

// It isn't needed for OSX, because it shows indicator by default
#if defined(QTOX_PLATFORM_EXT) && !defined(Q_OS_OSX)
#define ENABLE_CAPSLOCK_INDICATOR
#endif

CapsLockIndicator::EventHandler* CapsLockIndicator::eventHandler{nullptr};

CapsLockIndicator::CapsLockIndicator(QObject* parent) :
    QAction(parent)
{
#ifndef ENABLE_CAPSLOCK_INDICATOR
    setVisible(false);
#else
    setIcon(QIcon(":img/caps_lock.svg"));
    setToolTip(tr("CAPS-LOCK ENABLED"));

    if (!eventHandler)
        eventHandler = new EventHandler();
    eventHandler->actions.append(this);
#endif
}

CapsLockIndicator::~CapsLockIndicator()
{
#ifdef ENABLE_CAPSLOCK_INDICATOR
    eventHandler->actions.removeOne(this);
    if (eventHandler->actions.isEmpty())
    {
        delete eventHandler;
        eventHandler = nullptr;
    }
#endif
}

#ifdef ENABLE_CAPSLOCK_INDICATOR
CapsLockIndicator::EventHandler::EventHandler()
{
    QCoreApplication::instance()->installEventFilter(this);
}

CapsLockIndicator::EventHandler::~EventHandler()
{
    QCoreApplication::instance()->removeEventFilter(this);
}

void CapsLockIndicator::EventHandler::updateActions(const QObject* object)
{
    bool caps = Platform::capsLockEnabled();

    for (QAction* action : actions)
    {
        if (! object || object == action)
            action->setVisible(caps);
    }
}

bool CapsLockIndicator::EventHandler::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Show:
        updateActions(obj);
        break;
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
