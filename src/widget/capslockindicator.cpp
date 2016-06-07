#include "capslockindicator.h"
#ifdef QTOX_PLATFORM_EXT
#include "src/platform/capslock.h"
#endif

CapsLockIndicator::CapsLockIndicator(QWidget *parent) : QToolButton(parent)
{
    inputSize = QSize(130, 23);
    cleanInputStyle = parentWidget()->styleSheet();

    QIcon icon = QIcon(":img/caps_lock.svg");
    setIcon(icon);
    QSize iconSize(inputSize.height(), inputSize.height());
    setIconSize(iconSize);
    setCursor(Qt::ArrowCursor);
    move(inputSize.width() - inputSize.height(), 0);
    setStyleSheet("border: none; padding: 0; color: white");
    setToolTip(tr("CAPS-LOCK ENABLED"));
}

void CapsLockIndicator::show()
{
    QToolButton::show();

    QString style = QString("padding: -3px %1px -3px -6px; color: white").arg(iconSize().width() - 3);
    parentWidget()->setStyleSheet(style);
}

void CapsLockIndicator::hide()
{
    QToolButton::hide();
    parentWidget()->setStyleSheet(cleanInputStyle);
}

void CapsLockIndicator::updateIndicator()
{
    bool caps = false;
    // It doesn't needed for OSX, because it shows indicator by default
#if defined(QTOX_PLATFORM_EXT) && !defined(Q_OS_OSX)
    caps = Platform::capsLockEnabled();
#endif

    if (caps)
        show();
    else
        hide();
}
