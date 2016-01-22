#include "mainsettings.h"

MainSettings::MainSettings(QWidget *parent) : QWidget(parent)
{
    QWidget *frame3 = new QWidget(this);
    frame3->setGeometry(400, 400, 200, 200);
    frame3->setStyleSheet("background-image: url(:/vv_settings.png)");
}
