#include "mainwidget.h"
#include "mainsettings.h"
#include <QMouseEvent>
#include <math.h>

MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(0),
    texture(0),
    angularSpeed(0)
{
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete texture;
    delete geometries;
    doneCurrent();
}

/*
void MainWidget::mousePressEvent(QMouseEvent *e)
{
    // Save mouse press position
    mousePressPosition = QVector2D(e->localPos());
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e)
{
    // Mouse release position - mouse press position
    QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

    // Rotation axis is perpendicular to the mouse position difference
    // vector
    QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

    // Accelerate angular speed relative to the length of the mouse sweep
    qreal acc = diff.length() / 100.0;

    // Calculate new rotation axis as weighted sum
    rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();

    // Increase angular speed
    angularSpeed += acc;
}*/

void MainWidget::mousePressEvent(QMouseEvent *event)
{
    //QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
    //QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
    lastPos = event->pos();
    if (event->x() < 220 && event->y() <= 220 ) {
        //qDebug( "Pressed the invisi-button" );
        if (passIt == 1.0) {
            passIt = 0.0;
        }
        else {
            passIt = 1.0;
        }
        MainSettings ms(this);
        ms.show();
    update();
    }
    else if (event->x() < 220 && (event->y() > 220 && event->y() <= 440)) {
        //qDebug( "Pressed the invisi-button" );
        if (passIt == 1.0) {
            passIt = 0.0;
        }
        else {
            passIt = 1.0;
        }
        MainSettings ms(this);
        ms.show();
    update();
    }
    else if (event->x() < 220 && (event->y() > 440 && event->y() <= 660)) {
        //MainWidget widget;
        //widget.show();
        //qDebug( "Pressed the invisi-button" );
        if (passIt == 1.0) {
            passIt = 0.0;
        }
        else {
            passIt = 1.0;
        }
        MainSettings ms(this);
        ms.show();
    update();
    }
}

void MainWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        rotateBy(8 * dy, 8 * dx, 0);
    } else if (event->buttons() & Qt::RightButton) {
        rotateBy(8 * dy, 0, 8 * dx);
    }
    lastPos = event->pos();
}

void MainWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    emit clicked();
}





void MainWidget::rotateBy(int xAngle, int yAngle, int zAngle)
{
    xRot += xAngle;
    yRot += yAngle;
    zRot += zAngle;
    update();
}

void MainWidget::timerEvent(QTimerEvent *)
{
    // Decrease angular speed (friction)
    //UNUSED
    angularSpeed *= 0.99;

    // Stop rotation when speed goes below threshold
    if (angularSpeed < 0.01) {
        angularSpeed = 0.0;
    } else {
        // Update rotation
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;

        // Request an update
        update();
    }
}


void MainWidget::initializeGL()
{

    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();


    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);


    geometries = new GeometryEngine;


    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);
}


void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}


void MainWidget::initTextures()
{
    // Load cube.png image
    texture = new QOpenGLTexture(QImage(":/cube.png").mirrored());

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    texture->setWrapMode(QOpenGLTexture::Repeat);
}



void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}


void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture->bind();


    // Calculate model view transformation
    QMatrix4x4 matrix;
    matrix.translate(0.0, 0.0, -5.0);
    matrix.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    matrix.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    matrix.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);
    //matrix.rotate(rotation);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);
    program.setUniformValue("passIt", passIt);
    //QMatrix4x4 test = (passIt, 0.0, 0.0, 0.0);
    //program.setUniformValue("passIt", (passIt, 1.0, 1.0));


    // Use texture unit 0 which contains cube.png
    program.setUniformValue("texture", 0);

    // Draw cube geometry
    geometries->drawCubeGeometry(&program, passIt);

    QWidget *frame = new QWidget(this);
    frame->setGeometry(20, 0, 200, 200);
    frame->setStyleSheet("background-image: url(:/left_arrow.png)");

    QWidget *frame2 = new QWidget(this);
    frame2->setGeometry(20, 220, 200, 200);
    frame2->setStyleSheet("background-image: url(:/right_arrow.png)");

    QWidget *frame3 = new QWidget(this);
    frame3->setGeometry(20, 440, 200, 200);
    frame3->setStyleSheet("background-image: url(:/settings.png)");

    QWidget *frame4 = new QWidget(this);
    frame4->setGeometry(20, 660, 200, 200);
    frame4->setStyleSheet("background-image: url(:/contour.png)");
}
