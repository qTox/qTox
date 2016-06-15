#include "capslockindicator.h"
#ifdef QTOX_PLATFORM_EXT
#include "src/platform/capslock.h"
#endif
#include <QCoreApplication>

CapsLockIndicator::EventHandler* CapsLockIndicator::eventHandler{nullptr};

CapsLockIndicator::CapsLockIndicator(QObject* parent) :
    QAction(parent)
{
    setIcon(QIcon(":img/caps_lock.svg"));
    setToolTip(tr("CAPS-LOCK ENABLED"));

    if (!eventHandler)
        eventHandler = new EventHandler();
    eventHandler->actions.append(this);
}

CapsLockIndicator::~CapsLockIndicator()
{
    eventHandler->actions.removeOne(this);
    if (eventHandler->actions.isEmpty())
    {
        delete eventHandler;
        eventHandler = nullptr;
    }
}

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
    bool caps = false;
    // It doesn't needed for OSX, because it shows indicator by default
#if defined(QTOX_PLATFORM_EXT) && !defined(Q_OS_OSX)
    caps = Platform::capsLockEnabled();
#endif

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
