#include "capslockindicator.h"
#ifdef QTOX_PLATFORM_EXT
#include "src/platform/capslock.h"
#endif
#include <QCoreApplication>

CapsLockIndicator::CapsLockIndicator(QLineEdit *parent) :
    QAction(parent),
    parent(parent)
{
    setIcon(QIcon(":img/caps_lock.svg"));
    setToolTip(tr("CAPS-LOCK ENABLED"));

    QCoreApplication::instance()->installEventFilter(this);
}

void CapsLockIndicator::updateIndicator()
{
    bool caps = false;
    // It doesn't needed for OSX, because it shows indicator by default
#if defined(QTOX_PLATFORM_EXT) && !defined(Q_OS_OSX)
    caps = Platform::capsLockEnabled();
#endif

    if (caps)
        parent->addAction(this, QLineEdit::TrailingPosition);
    else
        parent->removeAction(this);
}

bool CapsLockIndicator::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Show:
        if (obj == this)
            updateIndicator();
        break;
    case QEvent::KeyRelease:
        updateIndicator();
        break;
    default:
        break;
    }

    return QAction::eventFilter(obj, event);
}
