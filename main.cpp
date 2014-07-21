#include "polyworldagent.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PolyworldAgent w;
    w.show();

    return a.exec();
}
