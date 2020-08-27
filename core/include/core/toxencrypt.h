/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

#include <QByteArray>
#include <QString>

#include <memory>

struct Tox_Pass_Key;

class ToxEncrypt
{
public:
    ~ToxEncrypt();
    ToxEncrypt() = delete;
    ToxEncrypt(const ToxEncrypt& other) = delete;
    ToxEncrypt& operator=(const ToxEncrypt& other) = delete;

    static int getMinBytes();
    static bool isEncrypted(const QByteArray& ciphertext);
    static QByteArray encryptPass(const QString& password, const QByteArray& plaintext);
    static QByteArray decryptPass(const QString& password, const QByteArray& ciphertext);
    static std::unique_ptr<ToxEncrypt> makeToxEncrypt(const QString& password);
    static std::unique_ptr<ToxEncrypt> makeToxEncrypt(const QString& password,
                                                      const QByteArray& toxSave);
    QByteArray encrypt(const QByteArray& plaintext) const;
    QByteArray decrypt(const QByteArray& ciphertext) const;

private:
    explicit ToxEncrypt(Tox_Pass_Key* key);

private:
    Tox_Pass_Key* passKey = nullptr;
};
