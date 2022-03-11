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

#pragma once

#include "src/core/toxencrypt.h"

#include <QDataStream>
#include <QSettings>
#include <QString>
#include <QVector>

class SettingsSerializer
{
public:
    enum class RecordTag : uint8_t
    {
        Value = 0,
        GroupStart = 1,
        ArrayStart = 2,
        ArrayValue = 3,
        ArrayEnd = 4,
    };
    SettingsSerializer(QString filePath_, const ToxEncrypt* passKey_ = nullptr);

    static bool isSerializedFormat(QString filePath);

    void load();
    void save();

    void beginGroup(const QString& prefix);
    void endGroup();

    int beginReadArray(const QString& prefix);
    void beginWriteArray(const QString& prefix, int size = -1);
    void endArray();
    void setArrayIndex(int i);

    void setValue(const QString& key, const QVariant& value);
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

private:
    struct Value
    {
        Value()
            : group{-2}
            , array{-2}
            , arrayIndex{-2}
            , key{QString()}
            , value{}
        {
        }
        Value(qint64 group_, qint64 array_, int arrayIndex_, QString key_, QVariant value_)
            : group{group_}
            , array{array_}
            , arrayIndex{arrayIndex_}
            , key{key_}
            , value{value_}
        {
        }
        qint64 group;
        qint64 array;
        int arrayIndex;
        QString key;
        QVariant value;
    };

    struct Array
    {
        qint64 group;
        int size;
        QString name;
        QVector<int> values;
    };

private:
    const Value* findValue(const QString& key) const;
    Value* findValue(const QString& key);
    void readSerialized();
    void readIni();
    void removeValue(const QString& key);
    void removeGroup(int group);
    void writePackedVariant(QDataStream& dataStream, const QVariant& v);

private:
    QString path;
    const ToxEncrypt* passKey;
    int group, array, arrayIndex;
    QStringList groups;
    QVector<Array> arrays;
    QVector<Value> values;
    static const char magic[];
};
