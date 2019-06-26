/*
    Copyright © 2015-2019 by The qTox Project Contributors

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


#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QMutex>
#include <QPair>
#include <QVector>
#include <functional>

class QTranslator;

class Translator
{
public:
    static void translate(const QString& localeName);
    static void registerHandler(const std::function<void()>&, void* owner);
    static void unregister(void* owner);

private:
    using Callback = QPair<void*, std::function<void()>>;
    static QVector<Callback> callbacks;
    static QMutex lock;
    static QTranslator* translator;
};

#endif // TRANSLATOR_H
