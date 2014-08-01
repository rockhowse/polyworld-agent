#pragma once

#include <list>
#include <ostream>
#include <string>

// polyworld includes
#include "gstage.h"

typedef std::list<class Monitor *> Monitors;
typedef std::list<class AgentTracker *> AgentTrackers;

class MonitorManager
{
 public:
    MonitorManager( class gstage &_mStage, std::string monitorPath );
	virtual ~MonitorManager();

	const Monitors &getMonitors();
	const AgentTrackers &getAgentTrackers();
	class AgentTracker *findAgentTracker( std::string name );

	void step();

	void dump( std::ostream &out );

 private:
	void addMonitor( class Monitor *monitor );
	void addAgentTracker( class AgentTracker *tracker );

	Monitors monitors;
	AgentTrackers agentTrackers;

    // stage to render to
    class gstage &mStage;
    class MonitorManager *monitorManager;
    int curStep;
};
