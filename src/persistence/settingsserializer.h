/*
    Copyright © 2015 by The qTox Project

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

#ifndef SETTINGSSERIALIZER_H
#define SETTINGSSERIALIZER_H

#include <QSettings>
#include <QVector>
#include <QString>
#include <QDataStream>

class SettingsSerializer
{
public:
    SettingsSerializer(QString filePath, const QString &password=QString());

    static bool isSerializedFormat(QString filePath);

    void load();
    void save();

    void beginGroup(const QString &prefix);
    void endGroup();

    int beginReadArray(const QString &prefix);
    void beginWriteArray(const QString &prefix, int size = -1);
    void endArray();
    void setArrayIndex(unsigned i);

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

private:
    enum class RecordTag : uint8_t
    {
        Value=0,
        GroupStart=1,
        ArrayStart=2,
        ArrayValue=3,
        ArrayEnd=4,
    };
    friend QDataStream& writeStream(QDataStream& dataStream, const SettingsSerializer::RecordTag& tag);
    friend QDataStream& readStream(QDataStream& dataStream, SettingsSerializer::RecordTag& tag);

    struct Value
    {
        Value() : group{-2},array{-2},key{QString()},value{}{}
        Value(qint64 group, qint64 array, qint64 arrayIndex, QString key, QVariant value)
            : group{group}, array{array}, arrayIndex{arrayIndex}, key{key}, value{value} {}
        qint64 group;
        qint64 array, arrayIndex;
        QString key;
        QVariant value;
    };

    struct Array
    {
        qint64 group;
        quint64 size;
        QString name;
        QVector<quint64> values;
    };

private:
    const Value *findValue(const QString& key) const;
    Value *findValue(const QString& key);
    void readSerialized();
    void readIni();
    void removeValue(const QString& key);
    void removeGroup(int group);
    void writePackedVariant(QDataStream& dataStream, const QVariant& v);

private:
    QString path;
    QString password;
    int group, array, arrayIndex;
    QVector<QString> groups;
    QVector<Array> arrays;
    QVector<Value> values;
    static const char magic[];
};

#endif // SETTINGSSERIALIZER_H
