#include "gui.h"
#include <QCoreApplication>
#include <QInputDialog>
#include <QThread>

GUI::GUI(QObject *parent) :
    QObject(parent)
{
}

GUI& GUI::getInstance()
{
    static GUI gui;
    return gui;
}

QString GUI::itemInputDialog(QWidget * parent, const QString & title,
                    const QString & label, const QStringList & items,
                    int current, bool editable, bool * ok,
                    Qt::WindowFlags flags,
                    Qt::InputMethodHints hints)
{
    if (QThread::currentThread() == qApp->thread())
    {
        return getInstance()._itemInputDialog(parent, title, label, items, current, editable, ok, flags, hints);
    }
    else
    {
        QString r;
        QMetaObject::invokeMethod(&getInstance(), "_itemInputDialog", Qt::BlockingQueuedConnection,
                    Q_RETURN_ARG(QString, r),
                    Q_ARG(QWidget*, parent), Q_ARG(const QString&, title),
                    Q_ARG(const QString&,label), Q_ARG(const QStringList&, items),
                    Q_ARG(int, current), Q_ARG(bool, editable), Q_ARG(bool*, ok),
                    Q_ARG(Qt::WindowFlags, flags), Q_ARG(Qt::InputMethodHints, hints));
        return r;
    }
}

QString GUI::_itemInputDialog(QWidget * parent, const QString & title,
                              const QString & label, const QStringList & items,
                              int current, bool editable, bool * ok,
                              Qt::WindowFlags flags,
                              Qt::InputMethodHints hints)
{
    return QInputDialog::getItem(parent, title, label, items, current, editable, ok, flags, hints);
}
