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

#include "settingsserializer.h"
#include "serialize.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/core/core.h"
#include <QSaveFile>
#include <QFile>
#include <QDebug>
#include <memory>
#include <cassert>

using namespace std;

const char SettingsSerializer::magic[] = {0x51,0x54,0x4F,0x58};

QDataStream& writeStream(QDataStream& dataStream, const SettingsSerializer::RecordTag& tag)
{
    return dataStream << static_cast<uint8_t>(tag);
}

QDataStream& writeStream(QDataStream& dataStream, const QByteArray& data)
{
    QByteArray size = vuintToData(data.size());
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
    return dataStream.operator >>((uint8_t&)tag);
}


QDataStream& readStream(QDataStream& dataStream, QByteArray& data)
{
    unsigned char num3;
    size_t num = 0;
    int num2 = 0;
    do
    {
        dataStream.readRawData((char*)&num3, 1);
        num |= (num3 & 0x7f) << num2;
        num2 += 7;
    } while ((num3 & 0x80) != 0);
    data.resize(num);
    dataStream.readRawData(data.data(), num);
    return dataStream;
}

SettingsSerializer::SettingsSerializer(QString filePath, const QString &password)
    : path{filePath}, password{password},
      group{-1}, array{-1}, arrayIndex{-1}
{
}

void SettingsSerializer::beginGroup(const QString &prefix)
{
    if (prefix.isEmpty())
        endGroup();
    int index = groups.indexOf(prefix);
    if (index >= 0)
    {
        group = index;
    }
    else
    {
        group = groups.size();
        groups.append(prefix);
    }
}

void SettingsSerializer::endGroup()
{
    group = -1;
}

int SettingsSerializer::beginReadArray(const QString &prefix)
{
    auto index = find_if(begin(arrays), end(arrays), [=](const Array& a){return a.name==prefix;});
    if (index != end(arrays))
    {
        array = index-begin(arrays);
        arrayIndex = -1;
        return index->size;
    }
    else
    {
        array = arrays.size();
        arrays.push_back({group, 0, prefix, {}});
        arrayIndex = -1;
        return 0;
    }
}

void SettingsSerializer::beginWriteArray(const QString &prefix, int size)
{
    auto index = find_if(begin(arrays), end(arrays), [=](const Array& a){return a.name==prefix;});
    if (index != end(arrays))
    {
        array = index-begin(arrays);
        arrayIndex = -1;
        if (size > 0)
            index->size = max(index->size, (quint64)size);
    }
    else
    {
        if (size < 0)
            size = 0;
        array = arrays.size();
        arrays.push_back({group, (uint64_t)size, prefix, {}});
        arrayIndex = -1;
    }
}

void SettingsSerializer::endArray()
{
    array = -1;
}

void SettingsSerializer::setArrayIndex(unsigned i)
{
    arrayIndex = i;
}

void SettingsSerializer::setValue(const QString &key, const QVariant &value)
{
    Value* v = findValue(key);
    if (v)
    {
        v->value = value;
    }
    else
    {
        Value nv{group, array, arrayIndex, key, value};
        if (array >= 0)
            arrays[array].values.append(values.size());
        values.append(nv);
    }
}

QVariant SettingsSerializer::value(const QString &key, const QVariant &defaultValue) const
{
    const Value* v = findValue(key);
    if (v)
        return v->value;
    else
        return defaultValue;
}

const SettingsSerializer::Value* SettingsSerializer::findValue(const QString& key) const
{
    if (array != -1)
    {
        for (const Array& a : arrays)
        {
            if (a.group != group)
                continue;
            for (int vi : a.values)
                if (values[vi].arrayIndex == arrayIndex && values[vi].key == key)
                    return &values[vi];
        }
    }
    else
    {
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

bool SettingsSerializer::isSerializedFormat(QString filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    char fmagic[8];
    if (f.read(fmagic, sizeof(fmagic)) != sizeof(fmagic))
        return false;
    return !memcmp(fmagic, magic, 4) || tox_is_data_encrypted((uint8_t*)fmagic);
}

void SettingsSerializer::load()
{
    if (isSerializedFormat(path))
        readSerialized();
    else
        readIni();
}

void SettingsSerializer::save()
{
    QSaveFile f(path);
    if (!f.open(QIODevice::Truncate | QIODevice::WriteOnly))
    {
        qWarning() << "Couldn't open file";
        return;
    }

    QByteArray data(magic, 4);
    QDataStream stream(&data, QIODevice::ReadWrite | QIODevice::Append);
    stream.setVersion(QDataStream::Qt_5_0);

    for (int g=-1; g<groups.size(); g++)
    {
        // Save the group name, if any
        if (g!=-1)
        {
            writeStream(stream, RecordTag::GroupStart);
            writeStream(stream, groups[g].toUtf8());
        }

        // Save all the arrays of this group
        for (const Array& a : arrays)
        {
            if (a.group != g)
                continue;
            if (a.size <= 0)
                continue;
            writeStream(stream, RecordTag::ArrayStart);
            writeStream(stream, a.name.toUtf8());
            writeStream(stream, vuintToData(a.size));

            for (uint64_t vi : a.values)
            {
                writeStream(stream, RecordTag::ArrayValue);
                writeStream(stream, vuintToData(values[vi].arrayIndex));
                writeStream(stream, values[vi].key.toUtf8());
                writePackedVariant(stream, values[vi].value);
            }
            writeStream(stream, RecordTag::ArrayEnd);
        }

        // Save all the values of this group that aren't in an array
        for (const Value& v : values)
        {
            if (v.group != g || v.array != -1)
                continue;
            writeStream(stream, RecordTag::Value);
            writeStream(stream, v.key.toUtf8());
            writePackedVariant(stream, v.value);
        }
    }

    // Encrypt
    if (!password.isEmpty())
    {
        Core* core = Nexus::getCore();
        auto passkey = core->createPasskey(password);
        data = core->encryptData(data, *passkey);
    }

    f.write(data);

    // check if everything got written
    if(f.flush())
    {
        f.commit();
    }
    else
    {
        f.cancelWriting();
        qCritical() << "Failed to write, can't save!";
    }
}

void SettingsSerializer::readSerialized()
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
    {
        qWarning() << "Couldn't open file";
        return;
    }
    QByteArray data = f.readAll();
    f.close();

    // Decrypt
    if (tox_is_data_encrypted((uint8_t*)data.data()))
    {
        if (password.isEmpty())
        {
            qCritical() << "The settings file is encrypted, but we don't have a password!";
            return;
        }

        Core* core = Nexus::getCore();

        uint8_t salt[TOX_PASS_SALT_LENGTH];
        tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);
        auto passkey = core->createPasskey(password, salt);

        data = core->decryptData(data, *passkey);
        if (data.isEmpty())
        {
            qCritical() << "Failed to decrypt the settings file";
            return;
        }
    }
    else
    {
        if (!password.isEmpty())
            qWarning() << "We have a password, but the settings file is not encrypted";
    }

    if (memcmp(data.data(), magic, 4))
    {
        qWarning() << "Bad magic!";
        return;
    }
    data = data.mid(4);

    QDataStream stream(&data, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_0);

    while (!stream.atEnd())
    {
        RecordTag tag;
        readStream(stream, tag);
        if (tag == RecordTag::Value)
        {
            QByteArray key;
            QByteArray value;
            readStream(stream, key);
            readStream(stream, value);
            setValue(QString::fromUtf8(key), QVariant(QString::fromUtf8(value)));
        }
        else if (tag == RecordTag::GroupStart)
        {
            QByteArray prefix;
            readStream(stream, prefix);
            beginGroup(QString::fromUtf8(prefix));
        }
        else if (tag == RecordTag::ArrayStart)
        {
            QByteArray prefix;
            readStream(stream, prefix);
            beginReadArray(QString::fromUtf8(prefix));
            QByteArray sizeData;
            readStream(stream, sizeData);
            if (sizeData.isEmpty())
            {
                qWarning("The personal save file is corrupted!");
                return;
            }
            quint64 size = dataToVUint(sizeData);
            arrays[array].size = max(size, arrays[array].size);
        }
        else if (tag == RecordTag::ArrayValue)
        {
            QByteArray indexData;
            readStream(stream, indexData);
            if (indexData.isEmpty())
            {
                qWarning("The personal save file is corrupted!");
                return;
            }
            quint64 index = dataToVUint(indexData);
            setArrayIndex(index);
            QByteArray key;
            QByteArray value;
            readStream(stream, key);
            readStream(stream, value);
            setValue(QString::fromUtf8(key), QVariant(QString::fromUtf8(value)));
        }
        else if (tag == RecordTag::ArrayEnd)
        {
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
        for (QString k : s.childKeys())
        {
            setValue(k, s.value(k));
            //qDebug() << "Read key "<<k<<" in group "<<group<<":\""<<groups[group]<<"\"";
        }

        // Add all groups
        gstack.push_back(QString());
        for (QString g : s.childGroups())
            gstack.push_back(g);

        // Visit the next group, if any
        while (!gstack.isEmpty())
        {
            QString g = gstack.takeLast();
            if (g.isEmpty())
            {
                if (gstack.isEmpty())
                    break;
                else
                    s.endGroup();
            }
            else
            {
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
    memset(groupSizes.get(), 0, groups.size()*sizeof(int));
    for (const Value& v : values)
    {
        if (v.group < 0 || v.group > groups.size())
            continue;
        groupSizes[v.group]++;
    }

    // Find arrays, remove their size key from the values, and add them to `arrays`
    QVector<int> groupsToKill;
    for (int i=values.size()-1; i>=0; i--)
    {
        const Value& v = values[i];
        if (v.group < 0 || v.group > groups.size())
            continue;
        if (groupSizes[v.group] != 1)
            continue;
        if (v.key != "size")
            continue;
        if (!v.value.canConvert(QVariant::Int))
            continue;

        Array a;
        a.size = v.value.toInt();
        int slashIndex = groups[v.group].lastIndexOf('/');
        if (slashIndex == -1)
        {
            a.group = -1;
            a.name = groups[v.group];
            a.size = v.value.toInt();
        }
        else
        {
            a.group = -1;
            for (int i=0; i<groups.size(); i++)
                if (groups[i] == groups[v.group].left(slashIndex))
                    a.group = i;
            a.name = groups[v.group].mid(slashIndex+1);

        }
        groupSizes[v.group]--;
        groupsToKill.append(v.group);
        arrays.append(a);
        values.removeAt(i);
        //qDebug() << "Found array"<<a.name<<"in group"<<a.group<<"size"<<a.size;
    }

    // Associate each array's values with the array
    for (int ai=0; ai<arrays.size(); ai++)
    {
        Array& a = arrays[ai];
        QString arrayPrefix;
        if (a.group != -1)
            arrayPrefix += groups[a.group]+'/';
        arrayPrefix += a.name+'/';

        // Find groups which represent each array index
        for (int g=0; g<groups.size(); g++)
        {
            if (!groups[g].startsWith(arrayPrefix))
                continue;
            bool ok;
            quint64 groupArrayIndex = groups[g].mid(arrayPrefix.size()).toInt(&ok);
            if (!ok)
                continue;
            groupsToKill.append(g);
            //qDebug() << "Found element"<<groupArrayIndex<<"of array"<<a.name;

            if (groupArrayIndex > a.size)
                a.size = groupArrayIndex;

            // Associate the values for this array index
            for (int vi=values.size()-1; vi>=0; vi--)
            {
                Value& v = values[vi];
                if (v.group != g)
                    continue;
                groupSizes[g]--;
                v.group = a.group;
                v.array = ai;
                v.arrayIndex = groupArrayIndex;
                a.values.append(vi);
                //qDebug() << "Found key"<<v.key<<"at index"<<groupArrayIndex<<"of array"<<a.name;
            }
        }
    }

    // Clean up spurious array element groups
    sort(begin(groupsToKill), end(groupsToKill), std::greater_equal<int>());
    for (int g : groupsToKill)
    {
        if (groupSizes[g])
            continue;
        //qDebug() << "Removing spurious array group"<<g<<groupSizes[g];
        removeGroup(g);
    }

    group = array = -1;
}

void SettingsSerializer::removeGroup(int group)
{
    assert(group<groups.size());
    for (Array& a : arrays)
    {
        assert(a.group != group);
        if (a.group > group)
            a.group--;
    }
    for (Value& v : values)
    {
        assert(v.group != group);
        if (v.group > group)
            v.group--;
    }
    groups.removeAt(group);
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
