#include "polyworldagent.h"
#include <QApplication>

#include "NetworkClient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PolyworldAgent w;
    w.show();

    NetworkClient client;
    client.show();

    return a.exec();
}
