#include "mainwidget.h"
#include "mainsettings.h"
#include <QMouseEvent>
#include <math.h>

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
    splash->setVisible(false);
    //QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
    //QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
    lastPos = event->pos();
    if (event->x() < 220 && event->y() <= 220 ) {
        if (passIt == 1.0) {
            passIt = 0.0;
        }
        else {
            passIt = 1.0;
        }
        //MainSettings ms(this);
        //ms.show();
    update();
    }
    else if (event->x() < 220 && (event->y() > 220 && event->y() <= 440)) {
        if (passIt == 1.0) {
            passIt = 0.0;
        }
        else {
            passIt = 1.0;
        }
        //MainSettings ms(this);
        //ms.show();
    update();
    }
    else if (event->x() < 220 && (event->y() > 440 && event->y() <= 660)) {
        qDebug("Pressed settings");
        if (toggleSettings == 1.0) {
            loadfile->setVisible(true);
            toggleSettings = 0.0;
        }
        else {
            loadfile->setVisible(false);
            toggleSettings = 1.0;
        }
        //MainSettings ms(this);
        //ms.show();
    update();
    }
    else if (event->x() < 220 && (event->y() > 660 && event->y() <= 880)) {
        if (toggleFOV == 1.0) {
            toggleFOV = 0.0;
        }
        else {
            toggleFOV = 1.0;
        }
        scale = scale + 0.1;
        //resizeGL(1920, 1080);
        //MainSettings ms(this);
        //ms.show();
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
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::Repeat);
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

    // Draw cube geometry
    geometries->drawCubeGeometry(&program, passIt);

    if (test != 1) {
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
/* BELOW IS SOME CODE THAT COULD BE VERY USEFUL TO OUR 3D TEXTURE STUFF
//============================================================================//
//                                 GLTexture3D                                //
//============================================================================//

GLTexture3D::GLTexture3D(int width, int height, int depth)
{
    GLBUFFERS_ASSERT_OPENGL("GLTexture3D::GLTexture3D", glTexImage3D, return)

    glBindTexture(GL_TEXTURE_3D, m_texture);
    glTexImage3D(GL_TEXTURE_3D, 0, 4, width, height, depth, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_3D, GL_GENERATE_MIPMAP, GL_TRUE);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void GLTexture3D::load(int width, int height, int depth, QRgb *data)
{
    GLBUFFERS_ASSERT_OPENGL("GLTexture3D::load", glTexImage3D, return)

    glBindTexture(GL_TEXTURE_3D, m_texture);
    glTexImage3D(GL_TEXTURE_3D, 0, 4, width, height, depth, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void GLTexture3D::bind()
{
    glBindTexture(GL_TEXTURE_3D, m_texture);
    glEnable(GL_TEXTURE_3D);
}

void GLTexture3D::unbind()
{
    glBindTexture(GL_TEXTURE_3D, 0);
    glDisable(GL_TEXTURE_3D);
}



//void VolumeViewer::BuildTexture(const char *ifile)
void MainWidget::BuildTexture(const char *ifile)
{
    // VERY IMPORTANT:
    // this line loads the address of the glTexImage3D function into the function pointer of the same name.
    // glTexImage3D is not implemented in the standard GL libraries and must be loaded dynamically at run time,
    // the environment the program is being run in MAY OR MAY NOT support it, if not we'll get back a NULL pointer.
    // this is necessary to use any OpenGL function declared in the glext.h header file
    // the Pointer to FunctioN ... PROC types are declared in the same header file with a type appropriate to the function name
    #ifdef WIN32
        glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
    #endif


    // ask for enough memory for the texels and make sure we got it before proceeding
    m_acTexVol.add(1);
    m_acTexVol.last() = (BYTE *)malloc(WIDTH * HEIGHT * DEPTH * BYTES_PER_TEXEL);
    if (m_acTexVol.last() == NULL)
        return;
    short int r, s, t;

    // each of the following loops defines one layer of our 3d texture, there are 3 unsigned bytes (red, green, blue) for each texel so each iteration sets 3 bytes
    // the memory pointed to by texels is technically a single dimension (C++ won't allow more than one dimension to be of variable length), the
    // work around is to use a mapping function like the one above that maps the 3 coordinates onto one dimension
    // layer 0 occupies the first (width * height * bytes per texel) bytes, followed by layer 1, etc...
    ifstream fin(ifile, ios::in | ios::binary);
    short int * aiTemp = new short int[ WIDTH * HEIGHT * DEPTH ];
    fin.read((char*) aiTemp, WIDTH * HEIGHT * DEPTH * sizeof(short int));
    int iIndex = 0;
    double dZeroIntensity = m_iWindowCenter - m_iWindowWidth/2.0;
    double dColorRange = 255.0;
    double dScaledIntensity = 0.0;
    for (r = 0; r < DEPTH; r++) {
        for (s = WIDTH-1; s >= 0; s--) {
        //<sandra>
        //for (s = 0; s < WIDTH; s++) {
            for (t = 0; t < HEIGHT; t++, iIndex++) {
            //for (t = HEIGHT-1; t >= 0; t--, iIndex++) {
                if (m_iWindowWidth <= 0)
                    dScaledIntensity = (double)aiTemp[iIndex];
                else
                    dScaledIntensity = ( ( ((double)aiTemp[iIndex]) - dZeroIntensity) / m_iWindowWidth)*dColorRange;
                if (dScaledIntensity < 0.0)
                    dScaledIntensity = 0.0;
                else if (dScaledIntensity > dColorRange)
                    dScaledIntensity = dColorRange;
                m_acTexVol.last()[TEXEL3(s, t, r)] = (BYTE)dScaledIntensity;
            }
        }
    }
    delete [] aiTemp;
    fin.close();

    // request 1 texture name from OpenGL
    m_agluiTexName.add(1);
    glGenTextures(1, &m_agluiTexName.last());
    // tell OpenGL we're going to be setting up the texture name it gave us
    glBindTexture(GL_TEXTURE_3D, m_agluiTexName.last());
    // when this texture needs to be shrunk to fit on small polygons, use linear interpolation of the texels to determine the color
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // when this texture needs to be magnified to fit on a big polygon, use linear interpolation of the texels to determine the color
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // we do not want the texture to repeat over the S axis, so if we specify coordinates out of range we will not get textured.
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    // same as above for T axis
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // same as above for R axis
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    // this is a 3d texture, level 0 (max detail), GL should store it in RGB8 format, its WIDTHxHEIGHTxDEPTH in size,
    // it doesnt have a border, we're giving it to GL in RGB format as a series of unsigned bytes, and texels is where the texel data is.
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, WIDTH, HEIGHT, DEPTH, 0, GL_RGB, GL_UNSIGNED_BYTE, texels);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, WIDTH, HEIGHT, DEPTH, 0, GL_RED, GL_UNSIGNED_BYTE, m_acTexVol.last());
}
*/

