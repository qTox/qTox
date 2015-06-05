#include "translator.h"
#include "src/misc/settings.h"
#include <QApplication>
#include <QString>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>

QTranslator* Translator::translator{nullptr};

void Translator::translate()
{
    if (!translator)
        translator = new QTranslator();

    // Load translations
    QCoreApplication::removeTranslator(translator);
    QString locale;
    if ((locale = Settings::getInstance().getTranslation()).isEmpty())
        locale = QLocale::system().name().section('_', 0, 0);

    if (locale == "en")
        return;

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
