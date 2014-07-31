#ifndef POLYWORLDAGENT_H
#define POLYWORLDAGENT_H

#include <QMainWindow>

// main stage used for rendering
#include "gstage.h"

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
    void initFromWorldFile();

    // used for rendering
    gstage fStage;

    class MonitorManager *monitorManager;

public slots:
    void appendStatus(const QString &statusText);
};

#endif // POLYWORLDAGENT_H
