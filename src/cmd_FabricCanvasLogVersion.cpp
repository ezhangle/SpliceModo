#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_ModoTools.h"
#include "cmd_FabricCanvasLogVersion.h"

// static tag description interface.
LXtTagInfoDesc FabricCanvasLogVersion::Command::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};

// constructor.
FabricCanvasLogVersion::Command::Command(void)
{
}

// execute code.
void FabricCanvasLogVersion::Command::cmd_Execute(unsigned flags)
{
  try
  {
    char s[1024];
    snprintf(s, sizeof(s), "plugin v. %.3f  /  core v. %s", FABRICMODO_PLUGIN_VERSION, FabricCore::GetVersionStr());
    feLog(s);
  }
  catch (FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
  }
}
