// ---------------------------------------------------------------
// 
// COMMAND:		HelloWorld.
// 
// ARGUMENTS:	none.
//
// NOTES:		- outputs "Hello World!" in the event log.
//				- does not use any Fabric stuff.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "cmd_HelloWorld.h"

// static thingy for the log system.
LXtTagInfoDesc cmd_HelloWorld::descInfo[] =
{
	{ LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
	{ 0 }
};

// execute code.
void cmd_HelloWorld::cmd_Execute(unsigned flags)
{
	gLog.Message(LXe_INFO, " ", "Hello World!", " ");
}
 
// end of file



