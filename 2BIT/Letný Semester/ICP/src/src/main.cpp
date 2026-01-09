/**
 * @file main.cpp
 * @author Eliška Křeménková (xkremee00)
 * @brief Main function of the project
 * @date 2025-05-09
 */

#include "../headers/mainwindow.h"
#include "../headers/comm_bridge.h"
#include <QApplication>
#include <QObject>

/**
 * @brief entry point of the program
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QObject::connect(&a, &QApplication::aboutToQuit, [&w](){
        CommBridge::goodbye();
    });

    return a.exec();
}
