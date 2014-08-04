#include "polyworldagent.h"
#include <QApplication>
#include <QtWidgets>

#include "NetworkClient.h"
#include "MulticastReceiver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PolyworldAgent polyWorldAgent;
    // only show this after we have connected
    //polyWorldAgent.show();

    NetworkClient networkClient;
    networkClient.show();

    // update status from networkclient
    QObject::connect(&networkClient, SIGNAL(setStatus(QString)), &polyWorldAgent, SLOT(appendStatus(QString)));

    // init world from world file after client has downloaded it
    QObject::connect(&networkClient, SIGNAL(startWorldInit()), &polyWorldAgent, SLOT(initFromWorldFile()));

    MulticastReceiver multicastReceiver;
    multicastReceiver.show();

    QObject::connect(&multicastReceiver, SIGNAL(setStatus(QString)), &polyWorldAgent, SLOT(appendStatus(QString)));
    QObject::connect(&multicastReceiver, SIGNAL(moveAgent(float,float,float,float)),
                     &polyWorldAgent, SLOT(moveAgent(float,float,float,float)));

    return a.exec();
}
