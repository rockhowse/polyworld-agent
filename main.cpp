#include "polyworldagent.h"
#include <QApplication>
#include <QtWidgets>

#include "NetworkClient.h"
#include "MulticastReceiver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PolyworldAgent polyWorldAgent;
    polyWorldAgent.show();

    NetworkClient networkClient;
    networkClient.show();

    QObject::connect(&networkClient, SIGNAL(setStatus(QString)), &polyWorldAgent, SLOT(appendStatus(QString)));

    MulticastReceiver multicastReceiver;
    multicastReceiver.show();

    QObject::connect(&multicastReceiver, SIGNAL(setStatus(QString)), &polyWorldAgent, SLOT(appendStatus(QString)));

    /*
    GetWorldFileClient  gwfc;
    gwfc.start("192.168.1.2",27000,"./feed_young.wf");
    */
    return a.exec();
}
