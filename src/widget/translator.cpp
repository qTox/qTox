/*
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "translator.h"
#include "src/persistence/settings.h"
#include <QApplication>
#include <QString>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>
#include <QMutexLocker>
#include <algorithm>

QTranslator* Translator::translator{nullptr};
QVector<Translator::Callback> Translator::callbacks;
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
            QString location = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
            if (qtTranslator->load(s_locale, location))
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

    // After the language is changed from RTL to LTR, the layout direction isn't
    // always restored
    const QString direction = QApplication::tr("LTR",
                 "Translate this string to the string 'RTL' in"
                 " right-to-left languages (for example Hebrew and"
                 " Arabic) to get proper widget layout");

    QGuiApplication::setLayoutDirection(
                direction == "RTL" ? Qt::RightToLeft : Qt::LeftToRight);

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
    callbacks.erase(std::remove_if(begin(callbacks), end(callbacks),
                    [=](const Callback& c){return c.first==owner;}), end(callbacks));
}
