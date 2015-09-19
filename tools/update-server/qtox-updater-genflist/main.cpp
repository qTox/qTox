/*
    Copyright © 2014 by The qTox Project

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
    along with qTox.  If not, see <http://www.gnu.org/licenses/>
*/

#include <iostream>
#include <QFile>
#include <QByteArray>
#include <QDir>
#include <QStack>
#include <QCryptographicHash>
#include <sodium.h>
#include "serialize.h"

using namespace std;

/// Pass the target folder as first argument, no spaces allowed. We'll call that dir $TARGET
/// Update the content of $TARGET/source/ before calling this tool
/// We'll generate $TARGET/flist and $TARGET/files/ then exit
/// We need qtox-updater-skey in our working directory to sign the flist
///
/// The generated flist is very simple and just installs everything in the working directory ...

QList<QString> scanDir(QDir dir)
{
    QList<QString> files;
    QStack<QString> stack;
    stack.push(dir.absolutePath());
    while (!stack.isEmpty())
    {
      QString sSubdir = stack.pop();
      QDir subdir(sSubdir);

      // Check for the files.
      QList<QString> sublist = subdir.entryList(QDir::Files);
      for (QString& file : sublist)
          file = dir.relativeFilePath(sSubdir + '/' + file);

      files += sublist;

      QFileInfoList infoEntries = subdir.entryInfoList(QStringList(),
                                                       QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
      for (int i = 0; i < infoEntries.size(); i++)
      {
         QFileInfo& item = infoEntries[i];
         stack.push(item.absoluteFilePath());
      }
    }
    return files;
}

int main(int argc, char* argv[])
{
    cout << "qTox updater flist generator" << endl;

    /// First some basic error handling, prepare our handles, ...
    if (argc != 2)
    {
        cout << "ERROR: qtox-updater-genflist takes the target path in argument" << endl;
        return 1;
    }

    QFile skeyFile("qtox-updater-skey");
    if (!skeyFile.open(QIODevice::ReadOnly))
    {
        cout << "ERROR: qtox-updater-genflist can't open the secret (private) key file" << endl;
        return 1;
    }
    QByteArray skeyData = skeyFile.readAll();
    skeyData = QByteArray::fromHex(skeyData);
    skeyFile.close();

    QString target(argv[1]);

    QFile flistFile(target+"/flist");
    if (!flistFile.open(QIODevice::Truncate | QIODevice::WriteOnly))
    {
        cout << "ERROR: qtox-updater-genflist can't open the target flist" << endl;
        return 1;
    }

    // Wipe the /files/ folder
    QDir(target+"/files/").removeRecursively();
    QDir(target).mkdir("files");

    QDir sdir(target+"/source/");
    if (!sdir.isReadable())
    {
        cout << "ERROR: qtox-updater-genflist can't open the target source directory" << endl;
        return 1;
    }

    QStringList filesListStr = scanDir(sdir);

    /// Serialize the flist data
    QByteArray flistData;
    for (QString fileStr : filesListStr)
    {
        cout << "Adding "<<fileStr.toStdString()<<"..."<<endl;

        QFile file(target+"/source/"+fileStr);
        if (!file.open(QIODevice::ReadOnly))
        {
            cout << "ERROR: qtox-updater-genflist couldn't open a target file to sign it" << endl;
            return 1;
        }

        QByteArray fileData = file.readAll();

        unsigned char sig[crypto_sign_BYTES];
        crypto_sign_detached(sig, nullptr, (unsigned char*)fileData.data(), fileData.size(), (unsigned char*)skeyData.data());

        QString id = QCryptographicHash::hash(fileStr.toUtf8(), QCryptographicHash::Sha3_224).toHex();

        flistData += QByteArray::fromRawData((char*)sig, crypto_sign_BYTES);
        flistData += stringToData(id);
        flistData += stringToData("./"+fileStr); ///< Always install in the working directory for now
        flistData += uint64ToData(fileData.size());

        file.close();
        if (!file.copy(target+"/files/"+id))
        {
            cout << "ERROR: qtox-updater-genflist couldn't copy target file to /files/" << endl;
            return 1;
        }
    }

    cout << "Signing and writing the flist..."<<endl;

    /// Sign our flist
    unsigned char sig[crypto_sign_BYTES];
    crypto_sign_detached(sig, nullptr, (unsigned char*)flistData.data(), flistData.size(), (unsigned char*)skeyData.data());

    /// Write the flist
    flistFile.write("1");
    flistFile.write((char*)sig, crypto_sign_BYTES);
    flistFile.write(flistData);

    flistFile.close();
    return 0;
}
