/*
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
#include <QMutex>
#include "src/video/videosource.h"

class QOpenGLBuffer;
class QOpenGLShaderProgram;

class VideoSurface : public QGLWidget
{
    Q_OBJECT

public:
    VideoSurface(QWidget* parent=0);
    VideoSurface(VideoSource* source, QWidget* parent=0);
    ~VideoSurface();

    void setSource(VideoSource* src); //NULL is a valid option

    // QGLWidget interface
protected:
    virtual void initializeGL();
    virtual void paintGL();

    void subscribe();
    void unsubscribe();

private slots:
    void onNewFrameAvailable(const VideoFrame &newFrame);

private:
    VideoSource* source;
    QOpenGLBuffer* pbo[2];
    QOpenGLShaderProgram* bgrProgramm;
    QOpenGLShaderProgram* yuvProgramm;
    GLuint textureId;
    int pboAllocSize;
    QSize res;
    bool hasSubscribed;

    QMutex mutex;
    VideoFrame frame;
    int pboIndex;
};

#endif // SELFCAMVIEW_H
