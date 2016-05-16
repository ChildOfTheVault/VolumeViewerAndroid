#include "mainwidget.h"
#include "mainsettings.h"
#include "math.h"
#include <sstream>


typedef uint8_t BYTE;
//#define WIDTH 512
//#define HEIGHT 512
//#define DEPTH 128
#define BYTES_PER_TEXEL 1
#define LAYER(r) (WIDTH * HEIGHT * r * BYTES_PER_TEXEL)
#define TEXEL2_MAX(s, t)	(BYTES_PER_TEXEL * (s * MAX_LEN + t))			// 2->1 dimension mapping function
#define TEXEL2(s, t)	(BYTES_PER_TEXEL * (s * WIDTH + t))			// 2->1 dimension mapping function
#define TEXEL3(s, t, r) (TEXEL2(s, t) + LAYER(r))					// 3->1 dimension mapping function

//QOpenGLTexture the_layers[DEPTH];

MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(0),
    texture(0),
    angularSpeed(0),
    thelayer(20),
    scale(1.0),
    scale_layer(0.15625),
    only_build_once(0),
    layer_image(),
    flatLayerData{0}
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

/*
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
*/


//The 'fake' origin is the origin for all rotation
//its actually the center of the cube
//this is the fake origin to *write* to
//in "write space"
const QVector2D FAKE_ORIGIN = QVector2D(MAX_HALFWAY, MAX_HALFWAY);
//this is the fake origin to read from (the volume data), in data space
const QVector3D DATA_FAKE_ORIGIN = QVector3D(HALFWAY(WIDTH), HALFWAY(HEIGHT), HALFWAY(DEPTH));
//this is our write origin, in write space
const QVector3D ORIGIN = QVector3D(0, 0, 0);
//this is our read origin, in data space
const QVector3D DATA_ORIGIN = QVector3D(0, 0, 0);
BYTE layerData[MAX_LEN][MAX_LEN] = {0};

uchar* MainWidget::calcCurrSlice(QQuaternion rotQuat){
    //volumeData is a "3D" array, with the setup depth, width, height
    //the write vector, used to choose which voxel is being written to
    QVector2D writeVect;
    //write vect, but in fake coords (from center, not from corner)
    //to center data
    QVector2D writeFakeVect;
    //read vect, in fake coords
    //to center rotation
    QVector3D readFakeVect;
    //rotated data vect
    //around center
    QVector3D readFakeTransformedVect;
    //rotated vect
    //but useable for reading from volume data
    QVector3D readTransformedVect;
    //read voxel
    BYTE voxel;


    //THE ALGORITHM
    //loop through every voxel in layerData
    //get fake write coordinates (to center the data!)
    //these fake write coords kinda correspond to read fake coords
    //now, rotate fake data coords (to rotate around the center!)
    //if the new vect is in bounds of data, get data, else = 0
    //the value at fake write coords is this value
    for(int i = 0; i < MAX_LEN; i++){
        for(int j = 0; j < MAX_LEN; j++){
            writeVect = QVector2D(i, j);
            //to write fake coords
            writeFakeVect = FAKE_ORIGIN - writeVect;
            //writeFakeVect.setX(FAKE_ORIGIN.x() + (-i));
            //writeFakeVect.setY(FAKE_ORIGIN.y() + (-j));

            //to read fake coords
            //by default, it reads the middle layer
            readFakeVect = QVector3D(writeFakeVect.x(), writeFakeVect.y(),0/*thelayer - DATA_FAKE_ORIGIN.z()*//*DATA_FAKE_ORIGIN.z()*/);
            //readFakeVect.setX(DATA_FAKE_ORIGIN.x() + (-i));
            //readFakeVect.setY(DATA_FAKE_ORIGIN.y() + (-j));
            //readFakeVect.setZ(DATA_FAKE_ORIGIN.z());

            //get read transformed fake coords
            readFakeTransformedVect = rotQuat.rotatedVector(readFakeVect);

            //add fake read origin to get real read coordinates
            readTransformedVect = DATA_FAKE_ORIGIN + readFakeTransformedVect;

            //check if its within volume bounds
            if((int)readTransformedVect.z() < 0 || (int)readTransformedVect.z() > DEPTH - 1){
                voxel = 127;
            } else if((int)readTransformedVect.x() < 0 || (int)readTransformedVect.x() > WIDTH - 1
                    || (int)readTransformedVect.y() < 0 || (int)readTransformedVect.y() > HEIGHT - 1){
                //out of bounds, its black = 0
                voxel = 255;
            } else {
                //its a weird format
                voxel = volumeData[(int)readTransformedVect.z()][(int)readTransformedVect.x()][(int)readTransformedVect.y()];
            }

            if(thelayer == 0){
                voxel = voxel * 1;
            }

            //write to it! unfortunately we need to keep the same format
            layerData[(int)writeVect.x()][(int)writeVect.y()] = (BYTE)voxel;
        }
    }


    for(unsigned int i = 0; i < MAX_LEN/*WIDTH*/; i++){
        for(unsigned int j = 0; j < MAX_LEN/*HEIGHT*/; j++){
            flatLayerData[TEXEL2_MAX(i, j)] = layerData[i][j];//volumeData[thelayer][i][j];
        }
    }


    return (uchar*)flatLayerData;
}

// direction -> true for up, false for down
// numSlices -> max # of slices to move up/down, default = 5
void MainWidget::moveCurrSlice(bool direction, int numSlices){
    int dirMult = direction ? 1 : -1;
    int layersToMove = dirMult * numSlices;

    if (toggleFOV == 1.0) {
        toggleFOV = 0.0;
    }
    else {
        toggleFOV = 1.0;
    }

    thelayer += layersToMove;

    if(thelayer >= DEPTH){
        thelayer = DEPTH - 1;
    } else if(thelayer < 0){
        thelayer = 0;
    }

    initTextures();
    //scale_layer = 0.15625 + thelayer * 0.0390625;
    //scale = scale + 0.1;
    //resizeGL(1920, 1080);
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
                 //qDebug("%d\n",testAttribute(Qt::WA_AcceptTouchEvents));
                 qDebug("%d\t%d\t%d\n",xRot, yRot, zRot);
                 xRot = 0;
                 yRot = 24;
                 zRot = 0;
                 passIt = 0.0;
                 passLock = 1.0;
                 scale = 1.5;
                 zoom_toggle = 0;
                 update();
             }
             else if (x < 220 && (y > 220 && y <= 440)) {
                 if (passIt == 0.0) {
                     passIt = 1.0;
                     scale = 1;
                     passLock = 0.0;
                 }
                 else {
                     passIt = 0.0;
                     //passLock = 1.0;
                     if (zoom_toggle == 0) {
                        passLock = 0.0;
                        scale = 1;
                        zoom_toggle = 1;
                     }
                     else {
                        passLock = 1.0;
                        scale = 1.5;
                        zoom_toggle = 0;
                     }
                 }
                 /*else {
                     passIt = 1.0;
                     passLock = 0.0;
                     scale = 1.0;
                 }*/
             update();
             }
             else if (x < 220 && (y > 440 && y <= 660)) {
                 moveCurrSlice(false);
                 update();
             }
             else if (x < 220 && (y > 660 && y <= 880)) {
                 moveCurrSlice(true);
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
            if (passLock != 1.0 && touch1.pos().x() > 220) {
                qreal x = touch1.pos().x();
                qreal y = touch1.pos().y();
                int dx = x - lastPos2.x();
                int dy = y - lastPos2.y();
                rotateBy(8 * dy, 8 * dx, 0);
                lastPos2 = touch1.pos();
            }
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
                 if (passLock != 1.0) {
                    if ((scale < 3.0 && currentScaleFactor < 1) || (scale > 0.4 && currentScaleFactor > 1)) {
                        scale *= ((1-currentScaleFactor)/25)+1;
                        update();
                    }
                 }
                 else {
                     //if ((thelayer < 250 && currentScaleFactor < 1) || (thelayer > 0 && currentScaleFactor > 1)) {
                     //if ((thelayer + ((((1-currentScaleFactor)/25)+1) * 3)) < 250 && (thelayer + ((((1-currentScaleFactor)/25)+1) * 3)) > 0) {
                        if (currentScaleFactor > 1 && thelayer < 127) {
                            thelayer++;
                            scale_layer += 0.0078125;
                        }
                        //double thescale= ((1-currentScaleFactor)/25)+1;
                        //thelayer = thelayer + (thescale * 3);
                        if (currentScaleFactor < 1 && thelayer > 0) {
                            thelayer--;
                            scale_layer -= 0.0078125;
                        }
                        initTextures();
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
        if (only_build_once == 0) {
            BuildTexture();
            only_build_once = 1;
            //the_layers = (QImage*) calloc(DEPTH, sizeof(QImage));
            //for (int o; o < DEPTH; o++) {
                //the_layers[o] = QImage((uchar*)m_acTexVol[o], WIDTH, HEIGHT, QImage::Format_Grayscale8);
                //the_layers[o] = QOpenGLTexture(q1);
            //}
        }
        //layer_image = QImage(calcCurrSlice(/*rotation*/), WIDTH, HEIGHT, QImage::Format_Grayscale8);
        //layer_image = QImage(calcCurrSlice(/*rotation*/), MAX_LEN, MAX_LEN, QImage::Format_Grayscale8);
        layer_image = QImage(calcCurrSlice(rotation), MAX_LEN, MAX_LEN, QImage::Format_Grayscale8);
        delete texture;
        texture = new QOpenGLTexture(layer_image);
        //texture = new QOpenGLTexture(the_layers[thelayer]);
    }

    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);

    //scale = 1;
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
    QMatrix4x4 rotMatrix;
    matrix.translate(0.0, 0.0, -5.0);
    rotMatrix.translate(0.0, 0.0, -5.0);
    rotMatrix.rotate(xRot / 25.0f, 1.0f, 0.0f, 0.0f); //Was 10.0f
    rotMatrix.rotate(yRot / 25.0f, 0.0f, 1.0f, 0.0f);
    rotMatrix.rotate(zRot / 25.0f, 0.0f, 0.0f, 1.0f);
    rotation = QQuaternion::fromRotationMatrix(rotMatrix.normalMatrix());
    double temp_s = scale*scale_layer;
    //printf("scale: %lf \t", temp_s);
    //qDebug("scale: %lf \t", temp_s);
    matrix.scale(scale,scale,temp_s);
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

void MainWidget::BuildTexture()
{
    //QOpenGLExtraFunctions *f = QOpenGLContext::currentContext()->extraFunctions();
    //											depth is 1 for first layer
    //int cols = WIDTH * HEIGHT * 1 * BYTES_PER_TEXEL;
    unsigned int lays = DEPTH;
    unsigned int cols = WIDTH;
    unsigned int rows = HEIGHT;

    volumeData = (BYTE ***) calloc(lays, sizeof(BYTE **));
    for(unsigned int i = 0; i < lays; i++){
        volumeData[i] = (BYTE **) calloc(cols, sizeof(BYTE *));
        for(unsigned int j = 0; j < cols; j++){
            volumeData[i][j] = (BYTE *) calloc(rows, sizeof(BYTE));
        }
    }
    if (volumeData == NULL)
        printf("NULL!");
    short int r, s, t;


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

    int iIndex = 0;
    double dColorRange = 255.0;
    double dScaledIntensity = 0.0;
    for (r = 0; r < DEPTH; r++) {
        for (s = WIDTH-1; s >= 0; s--) {
            for (t = 0; t < HEIGHT; t++, iIndex+=2) {
                dScaledIntensity = (double)blob[iIndex];//(double)aiTemp[iIndex];
                if (dScaledIntensity < 0.0)
                    dScaledIntensity = 0.0;
                else if (dScaledIntensity > dColorRange)
                    dScaledIntensity = dColorRange;
                //use 0 instead of r for just one layer
                volumeData[r][s][t] = (BYTE)dScaledIntensity;
            }
        }
    }
}


