#include "polyworldagent.h"
#include <QApplication>
#include <QtWidgets>

#include "NetworkClient.h"
//#include "GetWorldFileClient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PolyworldAgent polyWorldAgent;
    polyWorldAgent.show();


    NetworkClient networkClient;
    networkClient.show();


    QObject::connect(&networkClient, SIGNAL(setStatus()), &polyWorldAgent, SLOT(appendStatus()));

    /*
    GetWorldFileClient  gwfc;
    gwfc.start("192.168.1.2",27000,"./feed_young.wf");
    */
    return a.exec();
}
