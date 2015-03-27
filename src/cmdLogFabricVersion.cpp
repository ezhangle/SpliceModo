#include "plugin.h"
#include "cmdLogFabricVersion.h"

// static tag description interface.
LXtTagInfoDesc cmdLogFabricVersion::Command::descInfo[] =
{
	{ LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
	{ 0 }
};

// execute code.
void cmdLogFabricVersion::Command::cmd_Execute(unsigned flags)
{
	char s[1024];
	sprintf(s, "plugin version %.3f  /  core version %s", FABRICMODO_PLUGIN_VERSION, FabricCore::GetVersionStr());
	feLog(NULL, s, strlen(s));
}
 
// end of file



