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

class Camera;
class QOpenGLBuffer;
class QOpenGLShaderProgram;
class QTimer;

class SelfCamView : public QGLWidget
{
    Q_OBJECT

public:
    SelfCamView(Camera* Cam, QWidget *parent=0);
    ~SelfCamView();

    // QGLWidget interface
protected:
    virtual void initializeGL();
    virtual void paintGL();

    void update();

private:
    Camera* camera;
    QOpenGLBuffer* pbo;
    QOpenGLShaderProgram* program;
    QTimer* updateTimer;
    GLuint textureId;
    int pboAllocSize;
};

#endif // SELFCAMVIEW_H
