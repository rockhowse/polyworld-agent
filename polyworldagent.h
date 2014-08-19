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

    long fMateWait;

    // the client can only track a certain number of agents
    static const unsigned int maxAgents = 1024;

    // using an array instead of the sorted list for quick lookup
    agent * trackedAgents[maxAgents];

    // the client can only track a certain number of food objects
    static const unsigned int maxFood = 1024;

    // Food related properites
    float fFoodRemoveEnergy;

    // using an array instead of the sorted list for quick lookup
    food * trackedFood[maxFood];

    float fEat2Consume;

    int fStep;
    int numAgents;

    //
    int fSolidObjects;	// agents cannot pass through solid objects (collisions are avoided)
    int fCarryObjects;  // specifies which types of objects can be picked up.
    int fShieldObjects;  // specifies which types of objects act as shields.


public slots:
    void appendStatus(const QString &statusText);
    // called when the network client is finished downloading world file
    void initFromWorldFile();

private slots:
    void exeRender();
    void on_startPolyWin_clicked();

    // set server rotation and update global stats on step and agent numbers
    void serverStep(int serverStep,
                    int numAgents,
                    int numFood,
                    float sceneRotation);

    // needed to track agents
    void removeAgent(long agentNumber);

    void addAgent(long agentNumber,
                  float agentHeight,
                  float agentSize,
                  float maxSpeed);

    void drawAgentMove(long agentNumber,
                       float agentX,
                       float agentY,
                       float agentZ,
                       float agentYaw,
                       float agentRedChannel,
                       float agentGreenChannel,
                       float agentBluecChannel);

    // needed to track food
    void removeFood(long foodNumber);

    void addFood(long foodNumber,
                 float foodHeight,
                 float foodX,
                 float foodY,
                 float foodZ);

    void changeFood(long foodNumber,
                    float foodXLen,
                    float foodYLen,
                    float foodZLen);
    void on_actionTCPWindow_triggered();
    void on_actionMulticastWindow_triggered();
signals:
    void showTCPWindow();
    void showMulticastWindow();
};

#endif // POLYWORLDAGENT_H
