
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "geometryengine.h"
#include "paintwidget.h"

#include <qpainter.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>
#include <QLabel>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QPushButton>
#include <QScreen>
#include <QMouseEvent>
#include <QTouchEvent>
#include <math.h>
#include <cstdint>
#include <fstream>
#include <iostream>


typedef uint8_t BYTE;

#define WIDTH 512
#define HEIGHT 512
#define DEPTH 128

class GeometryEngine;

class MainWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    void rotateBy(int xAngle, int yAngle, int zAngle);
    ~MainWidget();

protected:
    //void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    //void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void initShaders();
    void initTextures();
    bool event(QEvent *event);
    void GLTexture3D(int width, int height, int depth);
    void BuildTexture();
    void load(int width, int height, int depth, QRgb *data);
    void bind();
    void unbind();
    void moveCurrSlice(bool direction, int numSlices=5);
    //void paintEvent(QPaintEvent *event);

signals:
    void clicked();

//private slots:
    //void handleButton();

private:
    QBasicTimer timer;
    QOpenGLShaderProgram program;
    GeometryEngine *geometries;
    QOpenGLTexture *texture;
    GLuint m_textures[DEPTH];
    QMatrix4x4 projection;
    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed;
    QQuaternion rotation;
    QPushButton *m_button;
    QPoint lastPos;
    QPointF lastPos2;
    QPointF lastPos3;
    int xRot;
    int yRot;
    int zRot;
    float passIt;
    float passLock;
    float toggleFOV;
    float toggleSettings;
    QWidget *loadfile;
    PaintWidget *paint;
    QWidget *splash;
    QWidget *frame4;
    int test;
    float scale;
    qreal totalScaleFactor;
    BYTE** m_acTexVol;
    int zoom_toggle;
    int mode_checker;
    float scale_layer;
    int only_build_once;
    //QImage *the_layers;
    QImage layer_image;
    int paint_mode;
    int paint_init;
    int toggle_load;
    qreal tempx;
    qreal tempy;
    //QPainter paint;
    int thelayer;
};


#endif // MAINWIDGET_H
