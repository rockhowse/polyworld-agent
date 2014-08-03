#ifndef POLYWORLDAGENT_H
#define POLYWORLDAGENT_H

#include <QMainWindow>

// main stage used for rendering
#include "gstage.h"
#include "gpolygon.h"
#include "MonitorView.h"

#include <QTimer>

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


    // used for rendering
    gstage fStage;
    TCastList fWorldCast;

    gpolyobj fGround;
    float fGroundClearance;
    Color fGroundColor;
    TSetList fWorldSet;

    QTimer renderTimer;

    class MonitorManager *monitorManager;
    MonitorViews monitorViews;

    void createMonitorViews();

    void InitGround();

    void addViewMenu( QMenuBar *menuBar );

public slots:
    void appendStatus(const QString &statusText);
    // called when the network client is finished downloading world file
    void initFromWorldFile();

private slots:
    void exeRender();
};

#endif // POLYWORLDAGENT_H
