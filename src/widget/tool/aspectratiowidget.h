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

#ifndef ASPECTRATIOWIDGET_H
#define ASPECTRATIOWIDGET_H

#include <QWidget>

class AspectRatioWidgetList;

class AspectRatioWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AspectRatioWidget(QWidget *parent = 0);
    float getRatio() const;
    bool aspectRatioRespected() const;
    virtual QSize sizeHint() const final override;
    void setSizeHint(int width, int height);
    void setMinimum(int minimumHieght);

protected:
    enum RatioMode
    {
        RespectWidth  = 0,
        RespectHeight = 1,
        MaximizeSize  = 2
    };

    void setRatio(float r);
    void setRatioMode(RatioMode mode);
    void setRatioWidth(int w = QWIDGETSIZE_MAX);

    friend class AspectRatioWidgetList;

private:
    RatioMode ratioMode;
    int ratioWidth;
    float ratio;
    QSize size_;
};

class AspectRatioWidgetList : public QVector<AspectRatioWidget*>
{
public:
    // Must update all simultaneously.
    void updateAll();
};

#endif // ASPECTRATIOWIDGET_H
