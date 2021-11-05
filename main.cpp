#include <iostream>
#include "simulatorinterface.h"

#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QColor>
#include <QPen>

int main(int argc, char *argv[])
{
    std::cout << "test" << std::endl;

    QApplication app(argc, argv);

    QPixmap map(128, 64);
    map.setDevicePixelRatio(0.25);
    map.fill(Qt::black);

    // Simulator sim;
    // sim.show();

    // label.show();

    QPainter painter(&map);


    QColor cyan("#00ffff");
    QPen pen(cyan, 1);
    painter.setPen(pen);
    QBrush brush;
    QColor color;
    color.setRgb(0, 255, 255);
    color.setAlpha(255);
    brush.setColor(color);
    painter.setBrush(brush);
    painter.drawLine(0, 0, 300, 300);
    painter.drawRect(10, 10, 100, 100);
    painter.end();

    QLabel label("test");
    label.setPixmap(map);
    
    label.show();
    return app.exec();
};