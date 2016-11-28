/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

#ifndef MASKABLEPIXMAPWIDGET_H
#define MASKABLEPIXMAPWIDGET_H

#include <QWidget>

class MaskablePixmapWidget final : public QWidget
{
    Q_OBJECT
public:
    MaskablePixmapWidget(QWidget *parent, QSize size, QString maskName = QString());
    ~MaskablePixmapWidget();
    void autopickBackground();
    void setClickable(bool clickable);
    void setPixmap(const QPixmap &pmap);
    QPixmap getPixmap() const;
    void setSize(QSize size);

signals:
    void clicked();

protected:
    virtual void paintEvent(QPaintEvent *) final override;
    virtual void mousePressEvent(QMouseEvent *) final override;

private:
    QPixmap pixmap, mask, unscaled;
    QPixmap* renderTarget;
    QSize size;
    QString maskName;
    bool clickable;
};

#endif // MASKABLEPIXMAPWIDGET_H
