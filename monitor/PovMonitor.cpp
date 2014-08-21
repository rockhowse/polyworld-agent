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
    agentPovRenderer = new AgentPovRenderer(50,
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
    // Update all agents, using their neurally controlled behaviors
    {
        agentPovRenderer->beginStep();

        // DECOUPLE START
        // don't need this for now since we have a single agent
        // and it doesn't have to "think"
        /*
        if( fStaticTimestepGeometry )
        {
            UpdateAgents_StaticTimestepGeometry();
        }
        else // if( fStaticTimestepGeometry )
        {
            UpdateAgents();
        }
        */
        //DECOUPLE END

        // Swap buffers for the agent POV window when they're all done
        agentPovRenderer->endStep();
    }
}
