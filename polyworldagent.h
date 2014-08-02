#ifndef POLYWORLDAGENT_H
#define POLYWORLDAGENT_H

#include <QMainWindow>

// main stage used for rendering
#include "gstage.h"
#include "gpolygon.h"

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
    TCastList fWorldCast;

    gpolyobj fGround;
    float fGroundClearance;
    Color fGroundColor;
    TSetList fWorldSet;

    class MonitorManager *monitorManager;

    void InitGround();

public slots:
    void appendStatus(const QString &statusText);
};

#endif // POLYWORLDAGENT_H
