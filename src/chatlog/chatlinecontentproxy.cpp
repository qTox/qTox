/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "chatlinecontentproxy.h"
#include "src/chatlog/content/filetransferwidget.h"
#include <QDebug>
#include <QLayout>
#include <QPainter>
#include <QWidget>

/**
 * @enum ChatLineContentProxy::ChatLineContentProxyType
 * @brief Type tag to avoid dynamic_cast of contained QWidget*
 *
 * @value GenericType
 * @value FileTransferWidgetType = 0
 */

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget, ChatLineContentProxyType type,
                                           int minWidth, float widthInPercent)
    : widthPercent(widthInPercent)
    , widthMin(minWidth)
    , widgetType{type}
{
    proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(widget);
}

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget, int minWidth, float widthInPercent)
    : ChatLineContentProxy(widget, GenericType, minWidth, widthInPercent)
{
}

ChatLineContentProxy::ChatLineContentProxy(FileTransferWidget* widget, int minWidth, float widthInPercent)
    : ChatLineContentProxy(widget, FileTransferWidgetType, minWidth, widthInPercent)
{
}

QRectF ChatLineContentProxy::boundingRect() const
{
    QRectF result = proxy->boundingRect();
    result.setHeight(result.height() + 5);
    return result;
}

void ChatLineContentProxy::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                                 QWidget* widget)
{
    painter->setClipRect(boundingRect());
    proxy->paint(painter, option, widget);
}

qreal ChatLineContentProxy::getAscent() const
{
    return 7.0;
}

QWidget* ChatLineContentProxy::getWidget() const
{
    return proxy->widget();
}

void ChatLineContentProxy::setWidth(float width)
{
    prepareGeometryChange();
    proxy->widget()->setFixedWidth(qMax(static_cast<int>(width * widthPercent), widthMin));
}

ChatLineContentProxy::ChatLineContentProxyType ChatLineContentProxy::getWidgetType() const
{
    return widgetType;
}
