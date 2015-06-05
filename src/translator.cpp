#include "translator.h"
#include "src/misc/settings.h"
#include <QApplication>
#include <QString>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>
#include <QMutexLocker>

QTranslator* Translator::translator{nullptr};
QVector<QPair<void*, std::function<void()>>> Translator::callbacks;
QMutex Translator::lock;

void Translator::translate()
{
    QMutexLocker locker{&lock};

    if (!translator)
        translator = new QTranslator();

    // Load translations
    QCoreApplication::removeTranslator(translator);
    QString locale;
    if ((locale = Settings::getInstance().getTranslation()).isEmpty())
        locale = QLocale::system().name().section('_', 0, 0);

    if (locale != "en")
    {
        if (translator->load(locale, ":translations/"))
        {
            qDebug() << "Loaded translation" << locale;

            // system menu translation
            QTranslator *qtTranslator = new QTranslator();
            QString s_locale = "qt_"+locale;
            if (qtTranslator->load(s_locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
            {
                QApplication::installTranslator(qtTranslator);
                qDebug() << "System translation loaded" << locale;
            }
            else
            {
                qDebug() << "System translation not loaded" << locale;
            }
        }
        else
        {
            qDebug() << "Error loading translation" << locale;
        }
        QCoreApplication::installTranslator(translator);
    }

    for (auto pair : callbacks)
        pair.second();
}

void Translator::registerHandler(std::function<void()> f, void *owner)
{
    QMutexLocker locker{&lock};
    callbacks.push_back({owner, f});
}

void Translator::unregister(void *owner)
{
    QMutexLocker locker{&lock};
    for (int i=0; i<callbacks.size(); i++)
    {
        if (callbacks[i].first == owner)
        {
            callbacks.removeAt(i);
            i--;
        }
    }
}
