#ifndef POLYWORLDAGENT_H
#define POLYWORLDAGENT_H

#include <QMainWindow>

// main stage used for rendering
#include "gstage.h"
#include "gpolygon.h"
#include "MonitorView.h"

#include <QTimer>
// needed for connect()
#include <QObject>

// needed to draw agents
# include <agent.h>

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

    class QTimer *renderTimer;

    class MonitorManager *monitorManager;
    MonitorViews monitorViews;

    void createMonitorViews();

    // initialize world file data
    void InitGround();
    void InitBarriers();

    void addViewMenu( QMenuBar *menuBar );

    // used to pull pertinante ground information from the worldfile
    void processWorldFile( proplib::Document *docWorldFile );

    // needed to track agents
    void addAgent();
    long fMateWait;
    agent* polyWorldAgent;

public slots:
    void appendStatus(const QString &statusText);
    // called when the network client is finished downloading world file
    void initFromWorldFile();
    void moveAgent(float agentX, float agentY, float agentZ, float agentYaw);

private slots:
    void exeRender();
};

#endif // POLYWORLDAGENT_H
