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

/// Serializes a QSettings's data in an (optionally) encrypted binary format
/// SettingsSerializer can detect regular .ini files and serialized ones,
/// it will read both regular and serialized .ini, but only save in serialized format.
/// The file is encrypted with the current profile's password, if any.
/// The file is only written to disk if save() is called, the destructor does not save to disk
/// All member functions are reentrant, but not thread safe.
class SettingsSerializer
{
public:
    SettingsSerializer(QString filePath, QString password=QString());

    static bool isSerializedFormat(QString filePath); ///< Check if the file is serialized settings. False on error.

    void load(); ///< Loads the settings from file
    void save(); ///< Saves the current settings back to file

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
        /// Followed by a QString key then a QVariant value
        Value=0,
        /// Followed by a QString group name
        GroupStart=1,
        /// Followed by a QString array name and a vuint array size
        ArrayStart=2,
        /// Followed by a vuint array index, a QString key then a QVariant value
        ArrayValue=3,
        /// Not followed by any data
        ArrayEnd=4,
    };
    friend QDataStream& operator<<(QDataStream& dataStream, const SettingsSerializer::RecordTag& tag);
    friend QDataStream& operator>>(QDataStream& dataStream, SettingsSerializer::RecordTag& tag);

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
    void removeGroup(int group); ///< The group must be empty
    void writePackedVariant(QDataStream& dataStream, const QVariant& v);

private:
    QString path;
    QString password;
    int group, array, arrayIndex;
    QVector<QString> groups;
    QVector<Array> arrays;
    QVector<Value> values;
    static const char magic[]; ///< Little endian ASCII "QTOX" magic
};

#endif // SETTINGSSERIALIZER_H
