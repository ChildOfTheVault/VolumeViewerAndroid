
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "geometryengine.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>
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
#define LONGEST (DEPTH > WIDTH ? (DEPTH > HEIGHT ? DEPTH : HEIGHT) : (WIDTH > HEIGHT ? WIDTH: HEIGHT))
#define HALFWAY(x) ((1.0 * x - 1) / 2)
//due to possible rotation configurations, the size of the data being shown
//might exceed a width of WIDTH and a height of HEIGHT.
//think about it being rotated on the z axis - the widest possible configuration
//is the length of the longest diagonal in the cube:
//longest_side * sqrt(3)
//by default, the value of a voxel is 0, which is completely black
//const int MAX_LEN = (int)(LONGEST * ceil(sqrt(3)));
//but for now we are doing a 2d layerData
//sqrt2 ~= 1.41 < 1.5 < ceil(sqrt2) == 2
//LONGEST * 1.5 = LONGEST + 0.5 * LONGEST = LONGEST + LONGEST / 2
#define MAX_LEN (LONGEST + (LONGEST / 2))
#define MAX_HALFWAY HALFWAY(MAX_LEN)

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
    //void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;
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
    uchar* calcCurrSlice(QQuaternion rotQuat=QQuaternion());

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
    QWidget *splash;
    int test;
    float scale;
    qreal totalScaleFactor;
    //BYTE flatLayerData[WIDTH * HEIGHT];
    BYTE flatLayerData[MAX_LEN * MAX_LEN];
    BYTE*** volumeData;
    int zoom_toggle;
    float scale_layer;
    int only_build_once;
    //QImage *the_layers;
    QImage layer_image;
    int thelayer;
};


#endif // MAINWIDGET_H
