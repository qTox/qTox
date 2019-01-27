/*
    Copyright © 2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is free software: you can redistribute it and/or modify
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

#ifndef STORNGTYPE_H
#define STORNGTYPE_H

#include <QHash>

/* This class facilitates creating a named class which wraps underlying POD,
 * avoiding implict casts and arithmetic of the underlying data.
 * Usage: Declare named type with arbitrary tag, then hook up Qt metatype for use
 * in signals/slots. For queued connections, registering the metatype is also
 * required before the type is used.
 *   using ReceiptNum = NamedType<uint32_t, struct ReceiptNumTag>;
 *   Q_DECLARE_METATYPE(ReceiptNum);
 *   qRegisterMetaType<ReceiptNum>();
 */

template <typename T, typename Parameter>
class NamedType
{
public:
    NamedType() {}
    explicit NamedType(T const& value) : value_(value) {}
    T& get() { return value_; }
    T const& get() const {return value_; }
    bool operator==(const NamedType& rhs) const { return value_ == rhs.value_; }
    bool operator<(const NamedType& rhs) const { return value_ < rhs.value_; }
    bool operator>(const NamedType& rhs) const { return value_ > rhs.value_; }

private:
    T value_;
};

template <typename T, typename Parameter>
inline uint qHash(const NamedType<T,Parameter> &key, uint seed = 0) {
    return qHash(key.get(), seed);
}
#endif // STORNGTYPE_H
