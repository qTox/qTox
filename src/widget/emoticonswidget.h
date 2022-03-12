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

#pragma once

#include <QMenu>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QVector>

#include <memory>

class QIcon;
class SmileyPack;
class Settings;
class Style;

class EmoticonsWidget : public QMenu
{
    Q_OBJECT
public:
    EmoticonsWidget(SmileyPack& smileyPack, Settings& settings, Style& style,
        QWidget* parent = nullptr);

signals:
    void insertEmoticon(QString str);

private slots:
    void onSmileyClicked();
    void onPageButtonClicked();
    void PageButtonsUpdate();

protected:
    void mouseReleaseEvent(QMouseEvent* ev) final;
    void mousePressEvent(QMouseEvent* ev) final;
    void wheelEvent(QWheelEvent* event) final;
    void keyPressEvent(QKeyEvent* e) final;

private:
    QStackedWidget stack;
    QVBoxLayout layout;
    QList<std::shared_ptr<QIcon>> emoticonsIcons;

public:
    QSize sizeHint() const override;
};
