#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_ModoTools.h"
#include "cmd_dfgLogVersion.h"

// static tag description interface.
LXtTagInfoDesc dfgLogVersion::Command::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};

// constructor.
dfgLogVersion::Command::Command(void)
{
}

// execute code.
void dfgLogVersion::Command::cmd_Execute(unsigned flags)
{
  char s[1024];
  snprintf(s, sizeof(s), "plugin v. %.3f  /  core v. %s", FABRICMODO_PLUGIN_VERSION, FabricCore::GetVersionStr());
  feLog(s);
}
