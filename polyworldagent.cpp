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

#define POLYWORLD_SCHEMA_FILE_NAME "/home/mint/polyworld-agent/etc/worldfile.wfs"
#define POLYWORLD_WORLD_FILE_NAME "feed_young.wf"


// Define directory mode mask the same, except you need execute privileges to use as a directory (go fig)
#define	PwDirMode ( S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH )

using namespace std;
using namespace genome;

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

void PolyworldAgent::initFromWorldFile()
{

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

