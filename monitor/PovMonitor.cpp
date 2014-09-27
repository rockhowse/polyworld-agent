#include "PovMonitor.h"
#include "Brain.h"

//===========================================================================
// PovMonitor
//===========================================================================
PovMonitor::PovMonitor()
     /*
       DECOUPLING
       TSimulation *_sim )
      : Monitor(POV, _sim, "pov", "POV", "POV")
       */
      :Monitor(POV, "pov", "POV", "POV"),
       agentPovRenderer(NULL)

{
    // because the client is only going to render the POV of itself
    // we are only going to have one agent to render
    agentPovRenderer = new AgentPovRenderer(20,
                                             Brain::config.retinaWidth,
                                             Brain::config.retinaHeight);
}

PovMonitor::~PovMonitor(){
    delete agentPovRenderer;
}

AgentPovRenderer *PovMonitor::getRenderer()
{
    //DECOUPLE
    // return sim->GetAgentPovRenderer();
    return agentPovRenderer;
}

void PovMonitor::step( long timestep )
{
    // I originally tried to render here
    // This has now been moved to the PolyworldAgent::UpdateAgents() function
    // this was done so we could iterate agents AND swap OpenGLContextes for rendering
}
