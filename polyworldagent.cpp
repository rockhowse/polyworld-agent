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

// Agent's POV view
#include "AgentPovRenderer.h"
#include "PovMonitor.h"
#include "PovMonitorView.h"

#define POLYWORLD_SCHEMA_FILE_NAME "/home/annihilatrix/polyworld-agent/etc/worldfile.wfs"
#define POLYWORLD_WORLD_FILE_NAME "/home/annihilatrix/build-polyworld-agent-Desktop_Qt_5_3_GCC_64bit-Debug/feed_young.wf"


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

    // needed by the AgentPOVRenderer
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
    monitorManager = new MonitorManager( fStage, string("/home/annihilatrix/polyworld-agent/etc/gui.mf"));

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
        case Monitor::STATUS_TEXT:
            {
                StatusTextMonitor *monitor = dynamic_cast<StatusTextMonitor *>( _monitor );
                view = new StatusTextMonitorView( monitor );
            }
            break;
         */
        case Monitor::POV:
            {
                PovMonitor *monitor = dynamic_cast<PovMonitor *>( _monitor );

                // set the agentPOVRenderer here need it to render during update agents
                agentPOVRenderer = monitor->getRenderer();
                view = new PovMonitorView( monitor );

                if(agentPOVRenderer) {
                    // add in notifications
                    QObject::connect(this, SIGNAL(agentBorn(agent *)),
                                     agentPOVRenderer, SLOT(add(agent *)));

                    QObject::connect(this, SIGNAL(agentDied(agent *)),
                                     agentPOVRenderer, SLOT(remove(agent *)));

                    QObject::connect(this, SIGNAL(agentRender(agent *)),
                                     agentPOVRenderer, SLOT(render(agent *)));
                }
            }
            break;
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

    // Process FoodTypes
    {
        food::gFoodHeight = doc.get( "FoodHeight" );
        food::gFoodColor = doc.get( "FoodColor" );
        food::gMinFoodEnergy = doc.get( "MinFoodEnergy" );
        food::gMaxFoodEnergy = doc.get( "MaxFoodEnergy" );
        food::gSize2Energy = doc.get( "FoodEnergySizeScale" );
        food::gCarryFood2Energy = doc.get( "EnergyUseCarryFood" );

        fEat2Consume = doc.get( "FoodConsumptionRate" );

        // needed for food
        globals::numEnergyTypes = doc.get( "NumEnergyTypes" );
        fFoodRemoveEnergy = doc.get( "FoodRemoveEnergy" );

        proplib::Property &propFoodTypes = doc.get( "FoodTypes" );
        int numFoodTypes = propFoodTypes.size();

        for( int ifoodType = 0; ifoodType < numFoodTypes; ifoodType++ )
        {
            proplib::Property &propFoodType = propFoodTypes.get( ifoodType );

            string name = propFoodType.get( "Name" );
            if( FoodType::lookup(name) != NULL )
            {
                propFoodType.err( "Duplicate name." );
            }

            Color foodColor;
            if( propFoodType.hasProperty("FoodColor") )
                foodColor = propFoodType.get( "FoodColor" );
            else
                foodColor = doc.get( "FoodColor" );
            EnergyPolarity energyPolarity = propFoodType.get( "EnergyPolarity" );
            Energy depletionThreshold = Energy::createDepletionThreshold( fFoodRemoveEnergy, energyPolarity );

            FoodType::define( name, foodColor, energyPolarity, depletionThreshold );
        }
    }

    // configure solid objects
    // not sure if client needs this or not...
    {
        fSolidObjects = 0;
#define __SET( PROP, MASK ) if( (bool)doc.get("Solid" PROP) ) fSolidObjects |= MASK##TYPE
        __SET( "Agents", AGENT );
        __SET( "Food", FOOD );
        __SET( "Bricks", BRICK );
#undef __SET
    }
}

// Initially this will only add a single agent to the follwing:
// 1. fStage
// 2. gXSortedObjects
void     PolyworldAgent::addAgent(long agentNumber, float agentHeight, float agentSize, float maxGeneCacheSpeed){

    // initially create a dummy agent
    trackedAgents[agentNumber] = agent::getfreeagent(&fStage);

    // don't do this as it resets the gene etc
    trackedAgents[agentNumber]->grow(fMateWait, agentSize, maxGeneCacheSpeed);

    float x = 0;
    //(fDomains[id].absoluteSizeX - 0.02) + fDomains[id].startX + 0.01;
    float z = 0;
    //(fDomains[id].absoluteSizeZ - 0.02) + fDomains[id].startZ + 0.01;
    float y = 0.5 * agent::config.agentHeight;
    float yaw = randpw() * 360.0;
    trackedAgents[agentNumber]->settranslation( x, y, z );
    trackedAgents[agentNumber]->setyaw( yaw );

    // add agent to stage
    fStage.AddObject(trackedAgents[agentNumber]);

    // add agent to list of objects
    objectxsortedlist::gXSortedObjects.add(trackedAgents[agentNumber]);

    // update the separationCache
    SeparationCache::birth(trackedAgents[agentNumber]);

    // notify components that agent has been born
    // 1. AgentPovRenderer
    emit agentBorn(trackedAgents[agentNumber]);

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

        // notify components the agent is dead
        // 1. AgentPOVRenderer
        emit agentDied(agentToDie);

        // remove agent from SeparationCache
        SeparationCache::death(agentToDie);

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

        // set the array value to 0
        // not exactly sure this is cosure in C++
        trackedAgents[agentNumber] = 0;
    }
}

// This moves a polyworld agent
void PolyworldAgent::drawAgentMove(long agentNumber,
                                   float agentX,
                                   float agentY,
                                   float agentZ,
                                   float agentYaw,
                                   float agentRedChannel,
                                   float agentGreenChannel,
                                   float agentBlueChannel) {

    if( trackedAgents[agentNumber]) {
        trackedAgents[agentNumber]->settranslation(agentX, agentY, agentZ);
        trackedAgents[agentNumber]->setyaw(agentYaw);
        trackedAgents[agentNumber]->SetRed(agentRedChannel);
        trackedAgents[agentNumber]->SetGreen(agentGreenChannel);
        trackedAgents[agentNumber]->SetBlue(agentBlueChannel);
    }
}

/**
 * This will currently update the view of all agents
 */
void PolyworldAgent::UpdateAgents() {

    // if we have instantiated the agentPovRenderer
    if(this->agentPOVRenderer) {

        // grab the GLContext()
        agentPOVRenderer->beginStep();

        agent* a;
        objectxsortedlist::gXSortedObjects.reset();
        while (objectxsortedlist::gXSortedObjects.nextObj(AGENTTYPE, (gobject**)&a))
        {
            // calculate view
            a->UpdateVision();

            // have the AgentPOVRenderer render it
            emit agentRender(a);

            //FIX ME
            //DECOUPLE
            /*
            a->UpdateBrain();
            if( !a->BeingCarried() )
                fFoodEnergyOut += a->UpdateBody(fMoveFitnessParameter,
                                                agent::config.speed2DPosition,
                                                fSolidObjects,
                                                NULL);
            */
        }

        // release GLContext
        agentPOVRenderer->endStep();
    }
}

// this is called every time a server step comes through
void PolyworldAgent::serverStep(int serverStep, int numAgents, int numFood, float sceneRotation) {

    fStep = serverStep;
    numAgents = numAgents;

    // update the agent POV views
    UpdateAgents();

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

    // see the .wf for current food types
    const FoodType *foodType = FoodType::lookup("Standard");

    food * newFood = new food( foodType, fStep );

    // initially create a dummey agent
    trackedFood[foodNumber] = newFood;
    trackedFood[foodNumber]->gFoodHeight = foodHeight;
    trackedFood[foodNumber]->setx(foodX);
    trackedFood[foodNumber]->sety(foodY);
    trackedFood[foodNumber]->setz(foodZ);
    //trackedFood[foodNumber]->setradius();

    // add agent to stage
    fStage.AddObject(trackedFood[foodNumber]);

    // add agent to list of objects
    objectxsortedlist::gXSortedObjects.add(trackedFood[foodNumber]);
}

/**
 *
 * This will remove a food object from the world.
 *
 * @brief PolyworldAgent::removeAgent
 * @param agentNumber
 */
void PolyworldAgent::removeFood(long foodNumber) {

    food * foodToRemove = trackedFood[foodNumber];

    if(foodToRemove){

        // ---
        // --- Remove From Stage
        // ---
        fStage.RemoveObject(foodToRemove);

        objectxsortedlist::gXSortedObjects.removeObjectWithLink( (gobject*) foodToRemove );

        delete foodToRemove;

        // set the array value to 0
        // not exactly sure this is cosure in C++
        trackedFood[foodNumber] = 0;
    }
}
/**
 *
 * During each step, we need to update the food object's size based on server state
 *
 * @brief PolyworldAgent::changeFood
 * @param foodNumber
 * @param foodXLen
 * @param foodYLen
 * @param foodZLen
 */
void PolyworldAgent::changeFood(long foodNumber,
                                float foodXLen,
                                float foodYLen,
                                float foodZLen) {
    food * foodToChange = trackedFood[foodNumber];

    if(foodToChange){
        foodToChange->setlen(foodXLen, foodYLen, foodZLen);
    }
}

void PolyworldAgent::on_startPolyWin_clicked()
{
    // load from existing world file
    initFromWorldFile();
}

void PolyworldAgent::on_actionTCPWindow_triggered()
{
    emit showTCPWindow();
}

void PolyworldAgent::on_actionMulticastWindow_triggered()
{
    emit showMulticastWindow();
}
