#include "gui.h"
#include "src/nexus.h"
#include <assert.h>
#include <QCoreApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QThread>

#ifdef Q_OS_ANDROID
#include "androidgui.h"
#else
#include "widget.h"
#endif

GUI::GUI(QObject *parent) :
    QObject(parent)
{
    assert(QThread::currentThread() == qApp->thread());

#ifndef Q_OS_ANDROID
    assert(Nexus::getDesktopGUI());
    connect(Nexus::getDesktopGUI(), &Widget::resized, this, &GUI::resized);
#endif
}

GUI& GUI::getInstance()
{
    static GUI gui;
    return gui;
}

// Implementation of the public clean interface

void GUI::setEnabled(bool state)
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._setEnabled(state);
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_setEnabled", Qt::BlockingQueuedConnection,
                                  Q_ARG(bool, state));
    }
}

void GUI::setWindowTitle(const QString& title)
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._setWindowTitle(title);
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_setWindowTitle", Qt::BlockingQueuedConnection,
                                  Q_ARG(const QString&, title));
    }
}

void GUI::reloadTheme()
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._reloadTheme();
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_reloadTheme", Qt::BlockingQueuedConnection);
    }
}

void GUI::showWarning(const QString& title, const QString& msg)
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._showWarning(title, msg);
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_showWarning", Qt::BlockingQueuedConnection,
                        Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

void GUI::showInfo(const QString& title, const QString& msg)
{
    if (QThread::currentThread() == qApp->thread())
    {
        getInstance()._showInfo(title, msg);
    }
    else
    {
        QMetaObject::invokeMethod(&getInstance(), "_showInfo", Qt::BlockingQueuedConnection,
                        Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

bool GUI::askQuestion(const QString& title, const QString& msg,
                            bool defaultAns, bool warning)
{
    if (QThread::currentThread() == qApp->thread())
    {
        return getInstance()._askQuestion(title, msg, defaultAns, warning);
    }
    else
    {
        bool ret;
        QMetaObject::invokeMethod(&getInstance(), "_askQuestion", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret),
                                  Q_ARG(const QString&, title), Q_ARG(const QString&, msg),
                                  Q_ARG(bool, defaultAns), Q_ARG(bool, warning));
        return ret;
    }
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

QString GUI::passwordDialog(const QString& cancel, const QString& body)
{
    if (QThread::currentThread() == qApp->thread())
    {
        return getInstance()._passwordDialog(cancel, body);
    }
    else
    {
        QString r;
        QMetaObject::invokeMethod(&getInstance(), "_passwordDialog", Qt::BlockingQueuedConnection,
                        Q_RETURN_ARG(QString, r),
                        Q_ARG(const QString&, cancel), Q_ARG(const QString&, body));
        return r;
    }
}

// Private implementations

void GUI::_setEnabled(bool state)
{
#ifdef Q_OS_ANDROID
    Nexus::getAndroidGUI()->setEnabled(state);
#else
    Nexus::getDesktopGUI()->setEnabled(state);
#endif
}

void GUI::_setWindowTitle(const QString& title)
{
    if (title.isEmpty())
        getMainWidget()->setWindowTitle("qTox");
    else
        getMainWidget()->setWindowTitle("qTox - " +title);
}

void GUI::_reloadTheme()
{
#ifndef Q_OS_ANDROID
    Nexus::getDesktopGUI()->reloadTheme();
#endif
}

void GUI::_showWarning(const QString& title, const QString& msg)
{
    QMessageBox::warning(getMainWidget(), title, msg);
}

void GUI::_showInfo(const QString& title, const QString& msg)
{
    QMessageBox::information(getMainWidget(), title, msg);
}

bool GUI::_askQuestion(const QString& title, const QString& msg,
                            bool defaultAns, bool warning)
{
    if (warning)
    {
        QMessageBox::StandardButton def = QMessageBox::Cancel;
        if (defaultAns)
            def = QMessageBox::Ok;
        return QMessageBox::warning(getMainWidget(), title, msg, QMessageBox::Ok | QMessageBox::Cancel, def) == QMessageBox::Ok;
    }
    else
    {
        QMessageBox::StandardButton def = QMessageBox::No;
        if (defaultAns)
            def = QMessageBox::Yes;
        return QMessageBox::question(getMainWidget(), title, msg, QMessageBox::Yes | QMessageBox::No, def) == QMessageBox::Yes;
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

QString GUI::_passwordDialog(const QString& cancel, const QString& body)
{
    // we use a hack. It is considered that closing the dialog without explicitly clicking
    // disable history is confusing. But we can't distinguish between clicking the cancel
    // button and closing the dialog. So instead, we reverse the Ok and Cancel roles,
    // so that nothing but explicitly clicking disable history closes the dialog
    QString ret;
    QInputDialog dialog;
    dialog.setWindowTitle(tr("Enter your password"));
    dialog.setOkButtonText(cancel);
    dialog.setCancelButtonText(tr("Decrypt"));
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextEchoMode(QLineEdit::Password);
    dialog.setLabelText(body);

    // problem with previous hack: the default button is disable history, not decrypt.
    // use another hack to reverse the default buttons.
    // http://www.qtcentre.org/threads/49924-Change-property-of-QInputDialog-button
    QList<QDialogButtonBox*> l = dialog.findChildren<QDialogButtonBox*>();
    if (!l.isEmpty())
    {
        QPushButton* ok     = l.first()->button(QDialogButtonBox::Ok);
        QPushButton* cancel = l.first()->button(QDialogButtonBox::Cancel);
        if (ok && cancel)
        {
            ok->setAutoDefault(false);
            ok->setDefault(false);
            cancel->setAutoDefault(true);
            cancel->setDefault(true);
        }
        else
            qWarning() << "PasswordDialog: Missing button!";
    }
    else
        qWarning() << "PasswordDialog: No QDialogButtonBox!";

    // using similar code, set QLabels to wrap
    for (auto* label : dialog.findChildren<QLabel*>())
        label->setWordWrap(true);

    while (true)
    {
        int val = dialog.exec();
        if (val == QDialog::Accepted)
            return QString();
        else
        {
            ret = dialog.textValue();
            if (!ret.isEmpty())
                return ret;
        }
        dialog.setTextValue("");
        dialog.setLabelText(body + "\n\n" + tr("You must enter a non-empty password:"));
    }
}

// Other

QWidget* GUI::getMainWidget()
{
    QWidget* maingui{nullptr};
#ifdef Q_OS_ANDROID
    maingui = Nexus::getAndroidGUI();
#else
    maingui = Nexus::getDesktopGUI();
#endif
    return maingui;
}
