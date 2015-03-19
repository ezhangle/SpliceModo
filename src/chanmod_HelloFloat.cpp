// ---------------------------------------------------------------
// 
// CHANNEL MODIFIER:		HelloFloat.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "chanmod_HelloFloat.h"

// static tag description interface.
LXtTagInfoDesc chanmod_HelloFloat::descInfo[] =
{
	{ LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
	{ 0 }
};

// execute code.
void chanmod_HelloFloat::cmd_Execute(unsigned flags)
{
	gLog.Message(LXe_INFO, " ", "Hello Float!", " ");
}
 
// end of file



