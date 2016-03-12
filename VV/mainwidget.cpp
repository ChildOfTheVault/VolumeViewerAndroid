#include "mainwidget.h"
#include "mainsettings.h"
#include <sstream>


typedef uint8_t BYTE;
#define WIDTH 512
#define HEIGHT 512
#define DEPTH 128
#define BYTES_PER_TEXEL 1
#define LAYER(r) (WIDTH * HEIGHT * r * BYTES_PER_TEXEL)
#define TEXEL2(s, t)	(BYTES_PER_TEXEL * (s * WIDTH + t))			// 2->1 dimension mapping function
#define TEXEL3(s, t, r) (TEXEL2(s, t) + LAYER(r))					// 3->1 dimension mapping function


MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(0),
    texture(0),
    angularSpeed(0)
    //toggleSettings(0),
    //toggleFOV(0)
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

bool MainWidget::event(QEvent *event)
 {
     switch (event->type()) {
     case QEvent::TouchBegin:
     {
         QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
         QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
         splash->setVisible(false);
         if (touchPoints.count() == 1) {
             const QTouchEvent::TouchPoint &touch1 = touchPoints.first();
             qreal x = touch1.pos().x();
             qreal y = touch1.pos().y();
             lastPos2 = touch1.lastPos();
             if (x < 220 && y <= 220 ) {
                 qDebug("%d\n",testAttribute(Qt::WA_AcceptTouchEvents));
                 update();
             }
             else if (x < 220 && (y > 220 && y <= 440)) {
                 if (passIt == 1.0) {
                     passIt = 0.0;
                 }
                 else {
                     passIt = 1.0;
                 }
             update();
             }
             else if (x < 220 && (y > 440 && y <= 660)) {
                 qDebug("Pressed settings");
                 if (toggleSettings == 1.0) {
                     loadfile->setVisible(true);
                     toggleSettings = 0.0;
                 }
                 else {
                     loadfile->setVisible(false);
                     toggleSettings = 1.0;
                 }
             update();
             }
             else if (x < 220 && (y > 660 && y <= 880)) {
                 if (toggleFOV == 1.0) {
                     toggleFOV = 0.0;
                 }
                 else {
                     toggleFOV = 1.0;
                 }
                 scale = scale + 0.1;
                 //resizeGL(1920, 1080);
             update();
             }
         }
     }
     case QEvent::TouchUpdate:
     {
         QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
         QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
         if (touchPoints.count() == 1) {
             const QTouchEvent::TouchPoint &touch1 = touchPoints.first();
            qreal x = touch1.pos().x();
            qreal y = touch1.pos().y();
            int dx = x - lastPos2.x();
            int dy = y - lastPos2.y();
            rotateBy(8 * dy, 8 * dx, 0);
            lastPos2 = touch1.pos();
         }
         if (touchPoints.count() == 2) {
             // determine scale factor
             const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
             const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
             qreal currentScaleFactor =
                     QLineF(touchPoint0.pos(), touchPoint1.pos()).length()
                     / QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
             //if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
             if ((lastPos2 != touchPoint0.pos() && lastPos3 != touchPoint1.pos())) {
                 // if one of the fingers is released, remember the current scale
                 // factor so that adding another finger later will continue zooming
                 // by adding new scale factor to the existing remembered value.
                 if ((scale < 3.0 && currentScaleFactor < 1) || (scale > 0.4 && currentScaleFactor > 1)) {
                    scale *= ((1-currentScaleFactor)/25)+1;
                    update();
                 }
                 currentScaleFactor = 1;
                 lastPos2 = touchPoint0.pos();
                 lastPos3 = touchPoint1.pos();
             }
             //setTransform(QTransform().scale(totalScaleFactor * currentScaleFactor,
                                             //totalScaleFactor * currentScaleFactor));
         }
     }
     case QEvent::TouchEnd:
     {
         QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
         QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
         return true;
     }
     default:
         break;
     }
     return QWidget::event(event);
 }

void MainWidget::initializeGL()
{

    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

    setAttribute(Qt::WA_AcceptTouchEvents);


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
    QFile file(":/parts/data_1");
    if (!file.exists()) {
        qDebug("FILE DOES NOT EXIST");
        // Load cube.png image
        texture = new QOpenGLTexture(QImage(":/cube.png").mirrored());
        file.close();
    }
    else {
        file.close();
        //pull out first layer of 3d data
        BuildTexture();

        //QImage layer1 = QImage((uchar*)m_acTexVol, WIDTH, HEIGHT, QImage::Format_Grayscale8);
        //texture = new QOpenGLTexture(layer1);
        texture = new QOpenGLTexture(QOpenGLTexture::Target3D);
        //texture->BindingTarget=QOpenGLTexture::BindingTarget3D;
        //texture->Feature=QOpenGLTexture::Texture3D;
        //texture->TextureFormat=QOpenGLTexture::Red;
        texture->setSize(WIDTH,HEIGHT,DEPTH);
        texture->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, (uchar*)m_acTexVol);
        qDebug("Depth: %d", texture->depth());
        qDebug("Width: %d", texture->width());
        qDebug("Height: %d", texture->height());

        //texture->PixelFormat=QOpenGLTexture::RGBA_Integer;
        //texture->PixelType=QOpenGLTexture::UInt8;

    }

    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);

    scale = 1;
}

void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    qreal zNear = 3.0, zFar = 7.0, fov = 45.0;
    /*if (toggleFOV == 1.0) {
        zNear = 3.0;
        zFar = 7.0;
        fov = 20.0;
    }*/


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
    matrix.rotate(xRot / 25.0f, 1.0f, 0.0f, 0.0f); //Was 10.0f
    matrix.rotate(yRot / 25.0f, 0.0f, 1.0f, 0.0f);
    matrix.rotate(zRot / 25.0f, 0.0f, 0.0f, 1.0f);
    matrix.scale(scale,scale,scale);
    //matrix.rotate(rotation);

    // Set modelview-projection matrix
    program.setUniformValue("mvp_matrix", projection * matrix);
    program.setUniformValue("passIt", passIt);
    //QMatrix4x4 test = (passIt, 0.0, 0.0, 0.0);
    //program.setUniformValue("passIt", (passIt, 1.0, 1.0));


    // Use texture unit 0 which contains cube.png
    program.setUniformValue("texture", 0);
    //glBindTexture(GL_TEXTURE_3D_OES, m_texture);


    // Draw cube geometry
    geometries->drawCubeGeometry(&program, passIt);

    if (test != 1) {
    QWidget *frame = new QWidget(this);
    frame->setGeometry(20, 0, 200, 200);
    frame->setStyleSheet("background-image: url(:/home.png)");

    QWidget *frame2 = new QWidget(this);
    frame2->setGeometry(20, 220, 200, 200);
    frame2->setStyleSheet("background-image: url(:/right_arrow.png)");

    QWidget *frame3 = new QWidget(this);
    frame3->setGeometry(20, 440, 200, 200);
    frame3->setStyleSheet("background-image: url(:/settings.png)");

    QWidget *frame4 = new QWidget(this);
    frame4->setGeometry(20, 660, 200, 200);
    frame4->setStyleSheet("background-image: url(:/contour.png)");

    //QWidget *loadfile = new QWidget(this);
      loadfile = new QWidget(this);
      loadfile->setGeometry(1600, 20, 180, 70);
      loadfile->setStyleSheet("background-image: url(:/button.png)");

      splash = new QWidget(this);
      splash->setGeometry(0, 0, 1920, 1080);
      splash->setStyleSheet("background-image: url(:/android/res/drawable-ldpi/logo.png)");

      test = 1;
    }
}

//BELOW IS SOME CODE THAT COULD BE VERY USEFUL TO OUR 3D TEXTURE STUFF
//============================================================================//
//                                 GLTexture3D                                //
//============================================================================//
/*
void MainWidget::GLTexture3D(int width, int height, int depth)
{
    //GLBUFFERS_ASSERT_OPENGL("MainWidget::GLTexture3D", glTexImage3D, return);
    //glTexImage3D
    //glBindTexture(m_texture);
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();

    GLuint test_tex[1];
    glGenTextures(1, test_tex);
    glBindTexture(GL_TEXTURE_3D_OES, test_tex[0]);
    f->glTexImage3D(GL_TEXTURE_3D_OES, 0, 4, width, height, depth, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_R_OES, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D_OES, GL_GENERATE_MIPMAP_HINT, GL_TRUE);
    glBindTexture(GL_TEXTURE_3D_OES, 0);
}

void MainWidget::load(int width, int height, int depth, QRgb *data)
{
    QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    glBindTexture(GL_TEXTURE_3D_OES, m_texture);
    f->glTexImage3D(GL_TEXTURE_3D_OES, 0, 4, width, height, depth, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_3D_OES, 0);
}

void MainWidget::bind()
{
    glBindTexture(GL_TEXTURE_3D_OES, m_texture);
    glEnable(GL_TEXTURE_3D_OES);
}

void MainWidget::unbind()
{
    glBindTexture(GL_TEXTURE_3D_OES, 0);
    glDisable(GL_TEXTURE_3D_OES);
}
*/
void MainWidget::BuildTexture()
{
    //QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    //											depth is 1 for first layer
    m_acTexVol = (BYTE *)malloc(WIDTH * HEIGHT * DEPTH * BYTES_PER_TEXEL);

    if (m_acTexVol == NULL)
        return;
    short int r, s, t;


    //short int aiTemp[WIDTH * HEIGHT * DEPTH * BYTES_PER_TEXEL];
    QByteArray blob;
    for (int i=1; i <= 64; i++) {
        std::ostringstream temp;
        temp << i;
        const QString temp2 = QString::fromStdString(":/parts/data_" + temp.str());
        QFile file(temp2);
        if (!file.exists()) {
            qDebug("FILE DOES NOT EXIST");
        } else {
            file.open(QIODevice::ReadOnly);
            blob.append(file.readAll());
        }

        file.close();
    }

    // each of the following loops defines one layer of our 3d texture, there are 3 unsigned bytes (red, green, blue) for each texel so each iteration sets 3 bytes
    // the memory pointed to by texels is technically a single dimension (C++ won't allow more than one dimension to be of variable length), the
    // work around is to use a mapping function like the one above that maps the 3 coordinates onto one dimension
    // layer 0 occupies the first (width * height * bytes per texel) bytes, followed by layer 1, etc...

    //int thelayer = 50;
    int iIndex = 0;
    //double dZeroIntensity = m_iWindowCenter - m_iWindowWidth/2.0;
    double dColorRange = 255.0;
    double dScaledIntensity = 0.0;
    for (r = 0; r < DEPTH; r++) {
        for (s = WIDTH-1; s >= 0; s--) {
        //for (s = 0; s < WIDTH; s++) {
            //for (t = 0; t < HEIGHT; t++, iIndex+=2) {
                //get last layer
                //if(r < thelayer){
                //    continue;
                //}
            for (t = HEIGHT-1; t >= 0; t--, iIndex++) {
                //if (m_iWindowWidth <= 0)
                dScaledIntensity = (double)blob[iIndex];//(double)aiTemp[iIndex];
                //else
                 //   dScaledIntensity = ( ( ((double)aiTemp[iIndex]) - dZeroIntensity) / m_iWindowWidth)*dColorRange;
                if (dScaledIntensity < 0.0)
                    dScaledIntensity = 0.0;
                else if (dScaledIntensity > dColorRange)
                    dScaledIntensity = dColorRange;
                //use 0 instead of r for just one layer
                m_acTexVol[TEXEL3(s, t, r)] = (BYTE)dScaledIntensity;
            }
        }
        //depth of 1 for first layer
        //if(r == thelayer){
        //    break;
        //}
    }

    // request 1 texture name from OpenGL
/*
    //m_agluiTexName.add(1);
    glGenTextures(1, &m_texture);
    // tell OpenGL we're going to be setting up the texture name it gave us
    glBindTexture(GL_TEXTURE_3D_OES, m_texture);
    // when this texture needs to be shrunk to fit on small polygons, use linear interpolation of the texels to determine the color
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // when this texture needs to be magnified to fit on a big polygon, use linear interpolation of the texels to determine the color
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // we do not want the texture to repeat over the S axis, so if we specify coordinates out of range we will not get textured.
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // same as above for T axis
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // same as above for R axis
    glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_R_OES, GL_CLAMP_TO_EDGE);
    // this is a 3d texture, level 0 (max detail), GL should store it in RGB8 format, its WIDTHxHEIGHTxDEPTH in size,
    // it doesnt have a border, we're giving it to GL in RGB format as a series of unsigned bytes, and texels is where the texel data is.
    //glTexImage3D(GL_TEXTURE_3D_OES, 0, GL_RGB8, WIDTH, HEIGHT, DEPTH, 0, GL_RGB, GL_UNSIGNED_BYTE, texels);
    //glTexImage3D(GL_TEXTURE_3D_OES, 0, GL_INTENSITY, WIDTH, HEIGHT, DEPTH, 0, GL_RED, GL_UNSIGNED_BYTE, m_acTexVol);
    f->glTexImage3D(GL_TEXTURE_3D_OES, 0, GL_R8_EXT, WIDTH, HEIGHT, DEPTH, 0, GL_RED_EXT, GL_UNSIGNED_BYTE, m_acTexVol);
    */
}


