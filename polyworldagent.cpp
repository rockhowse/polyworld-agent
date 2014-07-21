#include "polyworldagent.h"
#include "ui_polyworldagent.h"

PolyworldAgent::PolyworldAgent(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PolyworldAgent)
{
    ui->setupUi(this);
}

PolyworldAgent::~PolyworldAgent()
{
    delete ui;
}
