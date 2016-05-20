#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QtGui>
#include <QWidget>

class PaintWidget : public QWidget
{
    Q_OBJECT
public:
    int x1;
    int y1;
    int x2;
    int y2;
    int nlines;
   PaintWidget();
   //virtual ~PaintWidget();

protected:
    void paintEvent(QPaintEvent *event);
signals:

public slots:

private:
    int x1lines[1000];
    int y1lines[1000];
    int x2lines[1000];
    int y2lines[1000];

};


#endif // PAINTWIDGET_H
