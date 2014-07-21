#ifndef POLYWORLDAGENT_H
#define POLYWORLDAGENT_H

#include <QMainWindow>

namespace Ui {
class PolyworldAgent;
}

class PolyworldAgent : public QMainWindow
{
    Q_OBJECT

public:
    explicit PolyworldAgent(QWidget *parent = 0);
    ~PolyworldAgent();

private:
    Ui::PolyworldAgent *ui;
};

#endif // POLYWORLDAGENT_H
