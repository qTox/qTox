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

/**
@brief Loads the translations according to the settings or locale.
*/
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

/**
@brief Register a function to be called when the UI needs to be retranslated.
@param f Function, wich will called.
@param owner Widget to retanslate.
 */
void Translator::registerHandler(std::function<void()> f, void *owner)
{
    QMutexLocker locker{&lock};
    callbacks.push_back({owner, f});
}

/**
@brief Unregisters all handlers of an owner.
@param owner Owner to unregister.
*/
void Translator::unregister(void *owner)
{
    QMutexLocker locker{&lock};
    callbacks.erase(std::remove_if(begin(callbacks), end(callbacks),
                    [=](const Callback& c){return c.first==owner;}), end(callbacks));
}
