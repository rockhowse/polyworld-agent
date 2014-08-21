#ifndef POV_MONITOR_H
#define POV_MONITOR_H

#include "Monitor.h"
#include "AgentPovRenderer.h"

//===========================================================================
// PovMonitor
//===========================================================================
class PovMonitor : public Monitor
{
 public:
    //DECOUPLE
    //PovMonitor( class TSimulation *_sim );
    PovMonitor();
    virtual ~PovMonitor();

    class AgentPovRenderer *getRenderer();

    virtual void step( long timestep );

private:
    class AgentPovRenderer *agentPovRenderer;
};

#endif // POV_MONITOR_H
