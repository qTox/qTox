/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    
    This file is part of Tox Qt GUI.
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    
    See the COPYING file for more details.
*/

#ifndef ELIDELABEL_HPP
#define ELIDELABEL_HPP

#include <QLabel>

class ElideLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ElideLabel(QWidget *parent = 0);

    void setTextElide(bool set);
    bool textElide() const;

    void setTextElideMode(Qt::TextElideMode mode);
    Qt::TextElideMode textElideMode() const;

    void setShowToolTipOnElide(bool show);
    bool showToolTipOnElide();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    
private:
    bool _textElide;
    Qt::TextElideMode _textElideMode;

    bool _showToolTipOnElide;

    
};

#endif // ELIDELABEL_HPP
