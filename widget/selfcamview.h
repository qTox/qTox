/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef SELFCAMVIEW_H
#define SELFCAMVIEW_H

#include <QGLWidget>

class QOpenGLBuffer;
class QOpenGLShaderProgram;
class QTimer;
class VideoSource;

class SelfCamView : public QGLWidget
{
    Q_OBJECT

public:
    SelfCamView(VideoSource* source, QWidget* parent=0);
    ~SelfCamView();

    virtual void hideEvent(QHideEvent* ev);
    virtual void showEvent(QShowEvent* ev);

    // QGLWidget interface
protected:
    virtual void initializeGL();
    virtual void paintGL();
    virtual void updateGL();

    void update();

private:
    VideoSource* source;
    QOpenGLBuffer* pbo;
    QOpenGLShaderProgram* program;
    GLuint textureId;
    int pboAllocSize;
    QSize res;
    bool useNewFrame;
    bool hasSubscribed;

};

#endif // SELFCAMVIEW_H
