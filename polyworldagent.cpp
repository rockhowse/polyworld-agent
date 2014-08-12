#include <sys/stat.h>

#include "polyworldagent.h"
#include "ui_polyworldagent.h"

// Polyworld includes
#include "proplib.h"
#include "agent.h"
#include "GenomeSchema.h"
#include "Brain.h"
#include "SeparationCache.h"
#include "GenomeUtil.h"

// Polyworld graphics
#include "MonitorManager.h"
#include "Resources.h"
#include "SceneMonitorView.h"
#include "Monitor.h"
#include "CameraController.h"
#include "ToggleWidgetOpenAction.h"
#include "barrier.h"

#define POLYWORLD_SCHEMA_FILE_NAME "/home/mint/polyworld-agent/etc/worldfile.wfs"
#define POLYWORLD_WORLD_FILE_NAME "/home/mint/build-polyworld-agent-Desktop_Qt_5_3_0_GCC_64bit-Debug/feed_young.wf"


// Define directory mode mask the same, except you need execute privileges to use as a directory (go fig)
#define	PwDirMode ( S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH )

using namespace std;
using namespace genome;

PolyworldAgent::PolyworldAgent(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PolyworldAgent),
    renderTimer(new QTimer(this))
{
    ui->setupUi(this);
    // signal GL window to render
    connect(renderTimer, SIGNAL(timeout()), this, SLOT(exeRender()));

}



PolyworldAgent::~PolyworldAgent()
{
    delete ui;
}

void PolyworldAgent::initFromWorldFile()
{
    appendStatus("Starting to initialze from the world file.");
    srand(1);

    // initialize the python interpreter
    proplib::Interpreter::init_python();

    // ---
    // --- Create the run directory
    // ---
    {
        char s[256];
        char t[256];

        // First save the old directory, if it exists
        sprintf( s, "run" );
        sprintf( t, "run_%ld", time(NULL) );
        (void) rename( s, t );

        if( mkdir("run", PwDirMode) )
        {
            eprintf( "Error making run directory (%d)\n", errno );
            exit( 1 );
        }
    }

    // ---
    // --- Process the Worldfile
    // ---
    proplib::SchemaDocument *schema;
    proplib::Document *worldfile;
    {
        proplib::DocumentBuilder builder;
        schema = builder.buildSchemaDocument( POLYWORLD_SCHEMA_FILE_NAME );
        worldfile = builder.buildWorldfileDocument( schema, POLYWORLD_WORLD_FILE_NAME );

        {
            ofstream out( "run/converted.wf" );
            proplib::DocumentWriter writer( out );
            writer.write( worldfile );
        }

        schema->apply( worldfile );
    }

    // 1. Loads data needed to render the ground
    // 2. loads data needed to render the barriers
    processWorldFile( worldfile );
    agent::processWorldfile( *worldfile );
    GenomeSchema::processWorldfile( *worldfile );
    Brain::processWorldfile( *worldfile );

    // ---
    // --- General Init
    // ---
    Brain::init();
    agent::agentinit();
    SeparationCache::init();

    GenomeUtil::createSchema();

    // Pass ownership of the cast to the stage [TODO] figure out ownership issues
    fStage.SetCast(&fWorldCast);

    // initialize the ground objects; add them to fWorldSet
    InitGround();

    // initialize the barriers; add them to fWorldSet
    InitBarriers();

    // Add all initialized world objects to the stage for rendering
    fStage.SetSet(&fWorldSet);

    // ---
    // --- Init Monitors
    // ---
    monitorManager = new MonitorManager( fStage, string("/home/mint/polyworld-agent/etc/gui.mf"));

    createMonitorViews();
    show();

#if __APPLE__
    QMenuBar *menuBar = ::menuBar = new QMenuBar(0);
#else
    QMenuBar *menuBar = this->menuBar();
#endif
    addViewMenu( menuBar );

    // start the render timer
    renderTimer->start(0);
}

void PolyworldAgent::appendStatus(const QString &newStatus) {
    if(ui->statusEdit && !newStatus.isEmpty()){
        ui->statusEdit->append(newStatus);
    }
}

//---------------------------------------------------------------------------
// PolyworldAgent::InitGround
//---------------------------------------------------------------------------
void PolyworldAgent::InitGround()
{
    Resources::loadPolygons( &fGround, "ground" );

    fGround.sety(-fGroundClearance);
    fGround.setscale(globals::worldsize);
    fGround.setcolor(fGroundColor);
    fWorldSet.Add(&fGround);
}

//---------------------------------------------------------------------------
// PolyworldAgent::InitBarriers
//---------------------------------------------------------------------------
void PolyworldAgent::InitBarriers()
{
    // Add barriers
    barrier* b = NULL;
    while( barrier::gXSortedBarriers.next(b) )
        fWorldSet.Add(b);
}

void PolyworldAgent::createMonitorViews()
{
    citfor( Monitors, monitorManager->getMonitors(), it )
    {
        Monitor *_monitor = *it;
        MonitorView *view = NULL;

        switch( _monitor->getType() )
        {
        /*DECOUPLE
        case Monitor::CHART:
            {
                ChartMonitor *monitor = dynamic_cast<ChartMonitor *>( _monitor );
                view = new ChartMonitorView( monitor );
            }
            break;
        case Monitor::BRAIN:
            {
                BrainMonitor *monitor = dynamic_cast<BrainMonitor *>( _monitor );
                view = new BrainMonitorView( monitor );
            }
            break;
        case Monitor::POV:
            {
                PovMonitor *monitor = dynamic_cast<PovMonitor *>( _monitor );
                view = new PovMonitorView( monitor );
            }
            break;
        case Monitor::STATUS_TEXT:
            {
                StatusTextMonitor *monitor = dynamic_cast<StatusTextMonitor *>( _monitor );
                view = new StatusTextMonitorView( monitor );
            }
            break;
         */
        case Monitor::SCENE:
            {
                SceneMonitor *monitor = dynamic_cast<SceneMonitor *>( _monitor );
                view = new SceneMonitorView( monitor );
            }
            break;
        case Monitor::FARM:
            {
                // no-op
            }
            break;
        default:
            assert( false );
        }

        if( view )
        {
            if( (_monitor->getType() == Monitor::SCENE)
                && (centralWidget() == NULL) )
            {
                setCentralWidget( view );
            }

            view->loadSettings();

            monitorViews.push_back( view );
        }
    }
}

//---------------------------------------------------------------------------
// PolyworldAgent::addViewMenu
//---------------------------------------------------------------------------
void PolyworldAgent::addViewMenu( QMenuBar *menuBar )
{
    // View menu
    QMenu *menu = new QMenu( "&View", this );
    menuBar->addMenu( menu );

    itfor( MonitorViews, monitorViews, it )
    {
        MonitorView *view = *it;
        QAction *action = new ToggleWidgetOpenAction( this,
                                                      view,
                                                      view->getMonitor()->getName() );
        menu->addAction( action );
    }
}

void PolyworldAgent::exeRender()
{
    monitorManager->step();
}

void PolyworldAgent::processWorldFile(proplib::Document *docWorldFile) {

    proplib::Document &doc = *docWorldFile;

    //- Ground Config
    {
        fGroundColor = doc.get( "GroundColor" );
        fGroundClearance = doc.get( "GroundClearance" );
        globals::worldsize = doc.get( "WorldSize" );
    }

    //- Barrier Config
    {
        barrier::gBarrierHeight     = doc.get( "BarrierHeight" );
        barrier::gBarrierColor      = doc.get( "BarrierColor" );
        barrier::gStickyBarriers    = doc.get( "StickyBarriers" );
        barrier::gRatioPositions    = doc.get( "RatioBarrierPositions" );

        proplib::Property &propBarriers = doc.get( "Barriers" );

        for( int ibarrier = 0; ibarrier < (int)propBarriers.elements().size(); ibarrier++ )
        {
            proplib::Property &propBarrier = propBarriers.get( ibarrier );
            // Note that barriers were already allocated in InitCppProperties()
            barrier *b = new barrier();
            barrier::gBarriers.push_back( b );

            b->getPosition().xa = propBarrier.get( "X1" );
            b->getPosition().za = propBarrier.get( "Z1" );
            b->getPosition().xb = propBarrier.get( "X2" );
            b->getPosition().zb = propBarrier.get( "Z2" );

            b->init();

            barrier::gXSortedBarriers.add( b );
        }
    }

    //- Agent Config
    {
        fMateWait = doc.get( "MateWait" );
    }
}

// Initially this will only add a single agent to the follwing:
// 1. fStage
// 2. gXSortedObjects
void     PolyworldAgent::addAgent(long agentNumber, float agentHeight, float agentSize){

    // initially create a dummy agent
    trackedAgents[agentNumber] = agent::getfreeagent(&fStage);
    trackedAgents[agentNumber]->grow(fMateWait);

    float x = 0;
    //(fDomains[id].absoluteSizeX - 0.02) + fDomains[id].startX + 0.01;
    float z = 0;
    //(fDomains[id].absoluteSizeZ - 0.02) + fDomains[id].startZ + 0.01;
    float y = 0.5 * agent::config.agentHeight;
    float yaw = randpw() * 360.0;
    trackedAgents[agentNumber]->settranslation( x, y, z );
    trackedAgents[agentNumber]->setyaw( yaw );

    trackedAgents[agentNumber]->geneCache.size = 100.0;

    // add agent to stage
    fStage.AddObject(trackedAgents[agentNumber]);

    // add agent to list of objects
    objectxsortedlist::gXSortedObjects.add(trackedAgents[agentNumber]);
    //FIX-ME
    //newAgent->Domain(id);
    //fDomains[id].numAgents++;
    //fNewLifes++;
}

/**
 * This will remove an agent if it died on the server.
 *
 * @brief PolyworldAgent::removeAgent
 * @param agentNumber
 */
void PolyworldAgent::removeAgent(long agentNumber) {

    agent * agentToDie = trackedAgents[agentNumber];

    if(agentToDie){
        agentToDie->Die();

        // ---
        // --- Remove From Stage
        // ---
        fStage.RemoveObject(agentToDie);

        // ---
        // --- x-sorted list
        // ---
        // following assumes (requires!) list to be currently pointing to c,
        // and will leave the list pointing to the previous agent
        // agent::config.xSortedAgents.remove(); // get agent out of the list
        // objectxsortedlist::gXSortedObjects.removeCurrentObject(); // get agent out of the list

        // Following assumes (requires!) the agent to have stored c->listLink correctly
        objectxsortedlist::gXSortedObjects.removeObjectWithLink( (gobject*) agentToDie );

        delete agentToDie;
    }
}

// This moves a polyworld agent
void PolyworldAgent::drawAgentMove(long agentNumber, float agentX, float agentY, float agentZ, float agentYaw) {

    if( trackedAgents[agentNumber]) {
        trackedAgents[agentNumber]->settranslation(agentX, agentY, agentZ);
        trackedAgents[agentNumber]->setyaw(agentYaw);
    }
}

// this is called every time a server step comes through
void PolyworldAgent::serverStep(int serverStep, int numAgents, float sceneRotation) {
    //get scene rotation
    float newSceneRotation = 0.0;

    citfor( Monitors, monitorManager->getMonitors(), it )
    {
        Monitor *_monitor = *it;

        switch( _monitor->getType() )
        {
        case Monitor::SCENE:
            {
                SceneMonitor *monitor = dynamic_cast<SceneMonitor *>( _monitor );
                CameraController *_cameraController = monitor->getCameraController();
                _cameraController->setRotationAngle(sceneRotation);
            }
            break;
        }
    }
}

// Initially this will only add a single food to the follwing:
// 1. fStage
// 2. gXSortedObjects
void     PolyworldAgent::addFood(long foodNumber, float foodHeight, float foodX, float foodY, float foodZ){

    // initially create a dummy agent
    trackedAgents[agentNumber] = agent::getfreeagent(&fStage);
    trackedAgents[agentNumber]->grow(fMateWait);

    float x = 0;
    float z = 0;
    float y = 0.5 * agent::config.agentHeight;
    float yaw = randpw() * 360.0;

    trackedAgents[agentNumber]->settranslation( x, y, z );
    trackedAgents[agentNumber]->setyaw( yaw );
    trackedAgents[agentNumber]->geneCache.size = 100.0;

    // add agent to stage
    fStage.AddObject(trackedAgents[agentNumber]);

    // add agent to list of objects
    objectxsortedlist::gXSortedObjects.add(trackedAgents[agentNumber]);
    //FIX-ME
    //newAgent->Domain(id);
    //fDomains[id].numAgents++;
    //fNewLifes++;
}

void PolyworldAgent::on_startPolyWin_clicked()
{
    // load from existing world file
    initFromWorldFile();
}
