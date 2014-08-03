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
#include "ToggleWidgetOpenAction.h"

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

    // initialize the ground objects
    InitGround();

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
