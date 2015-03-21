// ---------------------------------------------------------------
// 
// COMMAND:     feGetFabricVersion.
// 
// ARGUMENTS:   none.
// 
// DESCRIPTION: outputs the Fabric Core and Splice versions
//              in the event log.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "command_feGetFabricVersion.h"

// static tag description interface.
LXtTagInfoDesc command_feGetFabricVersion::descInfo[] =
{
	{ LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
	{ 0 }
};

// execute code.
void command_feGetFabricVersion::cmd_Execute(unsigned flags)
{
	char s[256];
	sprintf(s, "Fabric Core version %s / Fabric Splice version %s", FabricSplice::GetFabricVersionStr(), FabricSplice::GetSpliceVersion());
	modoLog(s);
}
 
// end of file



