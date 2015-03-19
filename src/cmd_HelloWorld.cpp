// ---------------------------------------------------------------
// 
// COMMAND:		HelloWorld.
// 
// ARGUMENTS:	none.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "cmd_HelloWorld.h"

// static tag description interface.
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



