// ---------------------------------------------------------------
// 
// COMMAND:		feGetPluginVersion.
// 
// ARGUMENTS:	none.
// 
// DESCRIPTION:	outputs the plugin's version in the event log.
// 
// ---------------------------------------------------------------

#include "plugin.h"
#include "command_feGetPluginVersion.h"

// static tag description interface.
LXtTagInfoDesc command_feGetPluginVersion::descInfo[] =
{
	{ LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
	{ 0 }
};

// execute code.
void command_feGetPluginVersion::cmd_Execute(unsigned flags)
{
	char s[1024];
	sprintf(s, "FabricModo plugin version %.3f", FABRICMODO_PLUGIN_VERSION);
	feLog(NULL, s, strlen(s));
}
 
// end of file



