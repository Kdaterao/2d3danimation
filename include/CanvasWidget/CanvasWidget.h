// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef QGLWINDOW_H
#define QGLWINDOW_H


#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>


class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
public:
    explicit GLWidget(QWidget* parent = nullptr);

protected:
    void initializeGL() override; 
    void resizeGL(int w, int h) override;
    void paintGL() override;
};


#endif 