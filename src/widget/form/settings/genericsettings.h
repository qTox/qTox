/*
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

#ifndef GENERICFORM_H
#define GENERICFORM_H

#include <QWidget>

class GenericForm : public QWidget
{
    Q_OBJECT
public:
    explicit GenericForm(const QPixmap &icon) : formIcon(icon) {;}
    virtual ~GenericForm() {}

    virtual QString getFormName() = 0;
    QPixmap getFormIcon() {return formIcon;}

protected:
    QPixmap formIcon;

    template<typename Sender, typename Signal, typename... Rest>
    static QMetaObject::Connection connect_global_saver(const Sender sender, const Signal signal, const Rest... receiver);

    template<typename Sender, typename Signal, typename... Rest>
    static QMetaObject::Connection connect_personal_saver(const Sender sender, const Signal signal, const Rest... receiver);
};

// Because these are template functions they must be included into the header, otherwise
// the linker will not have references to the concrete instantiations of the template.
#include "genericsettings.tpp"

#endif
