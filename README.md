polyworld-client
================

This is the network client that connects to the polyworld-server. It's main function is to render the polyworld agent's retina and handle all local memory state of which the agent is aware. Unlike the original polyworld, the agent should be able to "think" independently. The initial concept is that the agent could run on a raspberry-pi allowing a lot of agents to run simultaneously.


Phase 1:
========
Connect to the polyworld-server and download the worldfile as we need this to render the world (agents/barriers etc.) as well as create our own agent.

Phase 2:
========
Pull in the source code for the polyworld agent and extract any non-agent dependencies

Phase 3:
========
Have agent be "created" and "destroyed" by "connecting" and "disconnecting" from the server.
