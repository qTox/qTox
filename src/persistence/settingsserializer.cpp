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

#include "settingsserializer.h"
#include "serialize.h"

#include "src/core/toxencrypt.h"
#include "src/persistence/profile.h"

#include <QDebug>
#include <QFile>
#include <QSaveFile>
#include <cassert>
#include <memory>

/**
 * @class SettingsSerializer
 * @brief Serializes a QSettings's data in an (optionally) encrypted binary format.
 * SettingsSerializer can detect regular .ini files and serialized ones,
 * it will read both regular and serialized .ini, but only save in serialized format.
 * The file is encrypted with the current profile's password, if any.
 * The file is only written to disk if save() is called, the destructor does not save to disk
 * All member functions are reentrant, but not thread safe.
 *
 * @enum SettingsSerializer::RecordTag
 * @var Value
 * Followed by a QString key then a QVariant value
 * @var GroupStart
 * Followed by a QString group name
 * @var ArrayStart
 * Followed by a QString array name and a vuint array size
 * @var ArrayValue
 * Followed by a vuint array index, a QString key then a QVariant value
 * @var ArrayEnd
 * Not followed by any data
 */
 /**
 * @var static const char magic[];
 * @brief Little endian ASCII "QTOX" magic
 */
const char SettingsSerializer::magic[] = {0x51, 0x54, 0x4F, 0x58};
namespace {

QDataStream& writeStream(QDataStream& dataStream, const SettingsSerializer::RecordTag& tag)
{
    return dataStream << static_cast<uint8_t>(tag);
}

QDataStream& writeStream(QDataStream& dataStream, const QByteArray& data)
{
    QByteArray size = vintToData(data.size());
    dataStream.writeRawData(size.data(), size.size());
    dataStream.writeRawData(data.data(), data.size());
    return dataStream;
}

QDataStream& writeStream(QDataStream& dataStream, const QString& str)
{
    return writeStream(dataStream, str.toUtf8());
}

QDataStream& readStream(QDataStream& dataStream, SettingsSerializer::RecordTag& tag)
{
    return dataStream >> reinterpret_cast<quint8&>(tag);
}


QDataStream& readStream(QDataStream& dataStream, QByteArray& data)
{
    char num3;
    int num = 0;
    int num2 = 0;
    do {
        dataStream.readRawData(&num3, 1);
        num |= (num3 & 0x7f) << num2;
        num2 += 7;
    } while ((num3 & 0x80) != 0);
    data.resize(num);
    dataStream.readRawData(data.data(), num);
    return dataStream;
}
} // namespace

SettingsSerializer::SettingsSerializer(QString filePath_, const ToxEncrypt* passKey_)
    : path{filePath_}
    , passKey{passKey_}
    , group{-1}
    , array{-1}
    , arrayIndex{-1}
{
}

void SettingsSerializer::beginGroup(const QString& prefix)
{
    if (prefix.isEmpty())
        endGroup();
    int index = groups.indexOf(prefix);
    if (index >= 0) {
        group = index;
    } else {
        group = groups.size();
        groups.append(prefix);
    }
}

void SettingsSerializer::endGroup()
{
    group = -1;
}

int SettingsSerializer::beginReadArray(const QString& prefix)
{
    auto index = std::find_if(std::begin(arrays), std::end(arrays),
                              [=](const Array& a) { return a.name == prefix; });

    if (index != std::end(arrays)) {
        array = static_cast<int>(index - std::begin(arrays));
        arrayIndex = -1;
        return index->size;
    } else {
        array = arrays.size();
        arrays.push_back({group, 0, prefix, {}});
        arrayIndex = -1;
        return 0;
    }
}

void SettingsSerializer::beginWriteArray(const QString& prefix, int size)
{
    auto index = std::find_if(std::begin(arrays), std::end(arrays),
                              [=](const Array& a) { return a.name == prefix; });

    if (index != std::end(arrays)) {
        array = static_cast<int>(index - std::begin(arrays));
        arrayIndex = -1;
        if (size > 0)
            index->size = std::max(index->size, size);
    } else {
        if (size < 0)
            size = 0;
        array = arrays.size();
        arrays.push_back({group, size, prefix, {}});
        arrayIndex = -1;
    }
}

void SettingsSerializer::endArray()
{
    array = -1;
}

void SettingsSerializer::setArrayIndex(int i)
{
    arrayIndex = i;
}

void SettingsSerializer::setValue(const QString& key, const QVariant& value)
{
    Value* v = findValue(key);
    if (v) {
        v->value = value;
    } else {
        Value nv{group, array, arrayIndex, key, value};
        if (array >= 0)
            arrays[array].values.append(values.size());
        values.append(nv);
    }
}

QVariant SettingsSerializer::value(const QString& key, const QVariant& defaultValue) const
{
    const Value* v = findValue(key);
    if (v)
        return v->value;
    else
        return defaultValue;
}

const SettingsSerializer::Value* SettingsSerializer::findValue(const QString& key) const
{
    if (array != -1) {
        for (const Array& a : arrays) {
            if (a.group != group)
                continue;

            for (int vi : a.values) {
                const Value& v = values[vi];
                if (v.arrayIndex == arrayIndex && v.key == key)
                    return &v;
            }
        }
    } else {
        for (const Value& v : values)
            if (v.group == group && v.array == -1 && v.key == key)
                return &v;
    }
    return nullptr;
}

SettingsSerializer::Value* SettingsSerializer::findValue(const QString& key)
{
    return const_cast<Value*>(const_cast<const SettingsSerializer*>(this)->findValue(key));
}

/**
 * @brief Checks if the file is serialized settings.
 * @param filePath Path to file to check.
 * @return False on error, true otherwise.
 */
bool SettingsSerializer::isSerializedFormat(QString filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    char fmagic[8];
    if (f.read(fmagic, sizeof(fmagic)) != sizeof(fmagic))
        return false;
    return !memcmp(fmagic, magic, 4) || tox_is_data_encrypted(reinterpret_cast<uint8_t*>(fmagic));
}

/**
 * @brief Loads the settings from file.
 */
void SettingsSerializer::load()
{
    if (isSerializedFormat(path))
        readSerialized();
    else
        readIni();
}

/**
 * @brief Saves the current settings back to file
 */
void SettingsSerializer::save()
{
    QSaveFile f(path);
    if (!f.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
        qWarning() << "Couldn't open file";
        return;
    }

    QByteArray data(magic, 4);
    QDataStream stream(&data, QIODevice::ReadWrite | QIODevice::Append);
    stream.setVersion(QDataStream::Qt_5_0);

    // prevent signed overflow and the associated warning
    int numGroups = std::max(0, groups.size());
    for (int g = -1; g < numGroups; ++g) {
        // Save the group name, if any
        if (g != -1) {
            writeStream(stream, RecordTag::GroupStart);
            writeStream(stream, groups[g].toUtf8());
        }

        // Save all the arrays of this group
        for (const Array& a : arrays) {
            if (a.group != g)
                continue;
            if (a.size <= 0)
                continue;
            writeStream(stream, RecordTag::ArrayStart);
            writeStream(stream, a.name.toUtf8());
            writeStream(stream, vintToData(a.size));

            for (int vi : a.values) {
                const Value& v = values[vi];
                writeStream(stream, RecordTag::ArrayValue);
                writeStream(stream, vintToData(values[vi].arrayIndex));
                writeStream(stream, v.key.toUtf8());
                writePackedVariant(stream, v.value);
            }
            writeStream(stream, RecordTag::ArrayEnd);
        }

        // Save all the values of this group that aren't in an array
        for (const Value& v : values) {
            if (v.group != g || v.array != -1)
                continue;
            writeStream(stream, RecordTag::Value);
            writeStream(stream, v.key.toUtf8());
            writePackedVariant(stream, v.value);
        }
    }

    // Encrypt
    if (passKey) {
        data = passKey->encrypt(data);
    }

    f.write(data);

    // check if everything got written
    if (f.flush()) {
        f.commit();
    } else {
        f.cancelWriting();
        qCritical() << "Failed to write, can't save!";
    }
}

void SettingsSerializer::readSerialized()
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file";
        return;
    }
    QByteArray data = f.readAll();
    f.close();

    // Decrypt
    if (ToxEncrypt::isEncrypted(data)) {
        if (!passKey) {
            qCritical() << "The settings file is encrypted, but we don't have a passkey!";
            return;
        }

        data = passKey->decrypt(data);
        if (data.isEmpty()) {
            qCritical() << "Failed to decrypt the settings file";
            return;
        }
    } else {
        if (passKey)
            qWarning() << "We have a password, but the settings file is not encrypted";
    }

    if (memcmp(data.data(), magic, 4)) {
        qWarning() << "Bad magic!";
        return;
    }
    data = data.mid(4);

    QDataStream stream(&data, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_0);

    while (!stream.atEnd()) {
        RecordTag tag;
        readStream(stream, tag);
        if (tag == RecordTag::Value) {
            QByteArray key;
            QByteArray value;
            readStream(stream, key);
            readStream(stream, value);
            setValue(QString::fromUtf8(key), QVariant(QString::fromUtf8(value)));
        } else if (tag == RecordTag::GroupStart) {
            QByteArray prefix;
            readStream(stream, prefix);
            beginGroup(QString::fromUtf8(prefix));
        } else if (tag == RecordTag::ArrayStart) {
            QByteArray prefix;
            readStream(stream, prefix);
            beginReadArray(QString::fromUtf8(prefix));
            QByteArray sizeData;
            readStream(stream, sizeData);
            if (sizeData.isEmpty()) {
                qWarning("The personal save file is corrupted!");
                return;
            }
            int size = dataToVInt(sizeData);
            arrays[array].size = qMax(size, arrays[array].size);
        } else if (tag == RecordTag::ArrayValue) {
            QByteArray indexData;
            readStream(stream, indexData);
            if (indexData.isEmpty()) {
                qWarning("The personal save file is corrupted!");
                return;
            }
            setArrayIndex(dataToVInt(indexData));
            QByteArray key;
            QByteArray value;
            readStream(stream, key);
            readStream(stream, value);
            setValue(QString::fromUtf8(key), QVariant(QString::fromUtf8(value)));
        } else if (tag == RecordTag::ArrayEnd) {
            endArray();
        }
    }

    group = array = -1;
}

void SettingsSerializer::readIni()
{
    QSettings s(path, QSettings::IniFormat);

    // Read all keys of all groups, reading arrays as raw keys
    QList<QString> gstack;
    do {
        // Add all keys
        if (!s.group().isEmpty())
            beginGroup(s.group());

        for (QString k : s.childKeys()) {
            setValue(k, s.value(k));
        }

        // Add all groups
        gstack.push_back(QString());
        for (QString g : s.childGroups())
            gstack.push_back(g);

        // Visit the next group, if any
        while (!gstack.isEmpty()) {
            QString g = gstack.takeLast();
            if (g.isEmpty()) {
                if (gstack.isEmpty())
                    break;
                else
                    s.endGroup();
            } else {
                s.beginGroup(g);
                break;
            }
        }
    } while (!gstack.isEmpty());

    // We can convert keys that look like arrays into real arrays
    // If a group's only key is called size, we'll consider it to be an array,
    // and its elements are all groups matching the pattern "[<group>/]<arrayName>/<arrayIndex>"

    // Find groups that only have 1 key
    std::unique_ptr<int[]> groupSizes{new int[groups.size()]};
    memset(groupSizes.get(), 0, static_cast<size_t>(groups.size()) * sizeof(int));
    for (const Value& v : values) {
        if (v.group < 0 || v.group > groups.size())
            continue;
        groupSizes[static_cast<size_t>(v.group)]++;
    }

    // Find arrays, remove their size key from the values, and add them to `arrays`
    QVector<int> groupsToKill;
    for (int i = values.size() - 1; i >= 0; i--) {
        const Value& v = values[i];
        if (v.group < 0 || v.group > groups.size())
            continue;
        if (groupSizes[static_cast<size_t>(v.group)] != 1)
            continue;
        if (v.key != "size")
            continue;
        if (!v.value.canConvert(QVariant::Int))
            continue;

        Array a;
        a.size = v.value.toInt();
        int slashIndex = groups[static_cast<int>(v.group)].lastIndexOf('/');
        if (slashIndex == -1) {
            a.group = -1;
            a.name = groups[static_cast<int>(v.group)];
            a.size = v.value.toInt();
        } else {
            a.group = -1;
            for (int j = 0; j < groups.size(); ++j)
                if (groups[j] == groups[static_cast<int>(v.group)].left(slashIndex))
                    a.group = j;
            a.name = groups[static_cast<int>(v.group)].mid(slashIndex + 1);
        }
        groupSizes[static_cast<size_t>(v.group)]--;
        groupsToKill.append(static_cast<int>(v.group));
        arrays.append(a);
        values.removeAt(i);
    }

    // Associate each array's values with the array
    for (int ai = 0; ai < arrays.size(); ++ai) {
        Array& a = arrays[ai];
        QString arrayPrefix;
        if (a.group != -1)
            arrayPrefix += groups[static_cast<int>(a.group)] + '/';
        arrayPrefix += a.name + '/';

        // Find groups which represent each array index
        for (int g = 0; g < groups.size(); ++g) {
            if (!groups[g].startsWith(arrayPrefix))
                continue;
            bool ok;
            int groupArrayIndex = groups[g].mid(arrayPrefix.size()).toInt(&ok);
            if (!ok)
                continue;
            groupsToKill.append(g);

            if (groupArrayIndex > a.size)
                a.size = groupArrayIndex;

            // Associate the values for this array index
            for (int vi = values.size() - 1; vi >= 0; vi--) {
                Value& v = values[vi];
                if (v.group != g)
                    continue;
                groupSizes[static_cast<size_t>(g)]--;
                v.group = a.group;
                v.array = ai;
                v.arrayIndex = groupArrayIndex;
                a.values.append(vi);
            }
        }
    }

    // Clean up spurious array element groups
    std::sort(std::begin(groupsToKill), std::end(groupsToKill), std::greater_equal<int>());

    for (int g : groupsToKill) {
        if (groupSizes[static_cast<size_t>(g)])
            continue;

        removeGroup(g);
    }

    group = array = -1;
}

/**
 * @brief Remove group.
 * @note The group must be empty.
 * @param group ID of group to remove.
 */
void SettingsSerializer::removeGroup(int group_)
{
    assert(group_ < groups.size());
    for (Array& a : arrays) {
        assert(a.group != group_);
        if (a.group > group_)
            a.group--;
    }
    for (Value& v : values) {
        assert(v.group != group_);
        if (v.group > group_)
            v.group--;
    }
    groups.removeAt(group_);
}

void SettingsSerializer::writePackedVariant(QDataStream& stream, const QVariant& v)
{
    assert(v.canConvert(QVariant::String));
    QString str = v.toString();
    if (str == "true")
        writeStream(stream, QString("1"));
    else if (str == "false")
        writeStream(stream, QString("0"));
    else
        writeStream(stream, str.toUtf8());
}
