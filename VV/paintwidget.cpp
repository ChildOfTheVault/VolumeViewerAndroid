#include "paintwidget.h"

PaintWidget::PaintWidget() {
    setAttribute(Qt::WA_TranslucentBackground);
    x1 = 0;
    x2 = 0;
    y1 = 0;
    y2 = 0;
    nlines = 0;
}
//PaintWidget::~PaintWidget() {}

void PaintWidget::paintEvent(QPaintEvent *event)
{
    //create a QPainter and pass a pointer to the device.
    //A paint device can be a QWidget, a QPixmap or a QImage
    //setAttribute(Qt::WA_TranslucentBackground);
    QPainter painter(this);

    painter.setPen(QPen(Qt::red, 4, Qt::SolidLine, Qt::RoundCap));
    x1lines[nlines] = x1;
    y1lines[nlines] = y1;
    x2lines[nlines] = x2;
    y2lines[nlines] = y2;
    for (int x = 0; x < nlines; x++)
        painter.drawLine(x1lines[x], y1lines[x], x2lines[x], y2lines[x]);
    nlines++;
    //a simple line
    /*painter.drawLine(1,1,100,100);

    //create a black pen that has solid line
    //and the width is 2.
    QPen myPen(Qt::black, 2, Qt::SolidLine);
    painter.setPen(myPen);
    painter.drawLine(100,100,100,1);

    //draw a point
    myPen.setColor(Qt::red);
    painter.drawPoint(110,110);

    //draw a polygon
    QPolygon polygon;
    polygon << QPoint(130, 140) << QPoint(180, 170)
             << QPoint(180, 140) << QPoint(220, 110)
             << QPoint(140, 100);
     painter.drawPolygon(polygon);

     //draw an ellipse
     //The setRenderHint() call enables antialiasing, telling QPainter to use different
     //color intensities on the edges to reduce the visual distortion that normally
     //occurs when the edges of a shape are converted into pixels
     painter.setRenderHint(QPainter::Antialiasing, true);
     painter.setPen(QPen(Qt::black, 3, Qt::DashDotLine, Qt::RoundCap));
     painter.setBrush(QBrush(Qt::green, Qt::SolidPattern));
     painter.drawEllipse(200, 80, 400, 240);*/

}

