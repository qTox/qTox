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

#include "selfcamview.h"
#include "camera.h"
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QDebug>

SelfCamView::SelfCamView(VideoSource *Source, QWidget* parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
    , source(Source)
    , pbo(nullptr)
    , program(nullptr)
    , textureId(0)
    , pboAllocSize(0)
    , uploadFrame(false)
    , hasSubscribed(false)
{
    qDebug()<<"NEW VideoSurface:"<<source->resolution();
    setFixedSize(source->resolution());
}

SelfCamView::~SelfCamView()
{
    if (pbo)
        delete pbo;

    if (textureId != 0)
        glDeleteTextures(1, &textureId);

    source->unsubscribe();
}

void SelfCamView::hideEvent(QHideEvent *ev)
{
    if (hasSubscribed)
    {
        source->unsubscribe();
        hasSubscribed = false;
        disconnect(source, &VideoSource::frameAvailable, this, &SelfCamView::updateGL);
    }

    QGLWidget::hideEvent(ev);
}

void SelfCamView::showEvent(QShowEvent *ev)
{
    if (!hasSubscribed)
    {
        source->subscribe();
        hasSubscribed = true;
        connect(source, &VideoSource::frameAvailable, this, &SelfCamView::updateGL);
    }

    QGLWidget::showEvent(ev);
}

void SelfCamView::initializeGL()
{

}

void SelfCamView::paintGL()
{
    if (!pbo)
    {
        qDebug() << "Creating pbo, program";

        // pbo
        pbo = new QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
        pbo->setUsagePattern(QOpenGLBuffer::StreamDraw);
        pbo->create();

        // shaders
        program = new QOpenGLShaderProgram;
        program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                         "attribute vec4 vertices;"
                                         "varying vec2 coords;"
                                         "void main() {"
                                         "    gl_Position = vec4(vertices.xy,0.0,1.0);"
                                         "    coords = vertices.xy*vec2(0.5,0.5)+vec2(0.5,0.5);"
                                         "}");
        program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                         "uniform sampler2D texture0;"
                                         "varying vec2 coords;"
                                         "void main() {"
                                         "    vec4 color = texture2D(texture0,coords*vec2(1.0, -1.0));"
                                         "    gl_FragColor = vec4(color.b, color.g, color.r, 1);"
                                         "}");

        program->bindAttributeLocation("vertices", 0);
        program->link();
    }

    if (res != source->resolution())
    {
        qDebug() << "Change resolution " << res << " to " << source->resolution();
        res = source->resolution();

        // a texture used to render the pbo (has the match the pixelformat of the source)
        glGenTextures(1,&textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D,0, GL_RGB, res.width(), res.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        setFixedSize(res);
    }


    if (uploadFrame)
    {
        source->lock();
        void* frame = source->getData();
        int frameBytes = source->getDataSize();

        if (pboAllocSize != frameBytes && frameBytes > 0)
        {
            qDebug() << "Resize pbo " << frameBytes << "bytes (was" << pboAllocSize << ") res " << source->resolution();

            pbo->bind();
            pbo->allocate(frameBytes);
            pbo->release();

            pboAllocSize = frameBytes;
        }

        // transfer data
        pbo->bind();

        void* ptr = pbo->map(QOpenGLBuffer::WriteOnly);
        if (ptr && frame)
            memcpy(ptr, frame, frameBytes);
        pbo->unmap();

        source->unlock();

        //transfer pbo data to texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0, res.width(), res.height(), GL_RGB, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        pbo->release();

        uploadFrame = false;
    }

    // render pbo
    float values[] = {
        -1, -1,
        1, -1,
        -1, 1,
        1, 1
    };

    program->setAttributeArray(0, GL_FLOAT, values, 2);


    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, width(), height());

    program->bind();
    program->enableAttributeArray(0);

    glBindTexture(GL_TEXTURE_2D, textureId);

    //draw fullscreen quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    program->disableAttributeArray(0);
    program->release();
}

void SelfCamView::updateGL()
{
    uploadFrame = true;
    QGLWidget::updateGL();
}


