#include "plugin.h"

#include "_class_ModoTools.h"
#include "_class_DFGUICmdHandlerDCC.h"
#include "cmd_FabricCanvasGetResult.h"

// static tag description interface.
LXtTagInfoDesc FabricCanvasGetResult::Command::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};

// constructor.
FabricCanvasGetResult::Command::Command(void)
{
  // arguments.
  int idx = 0;
  {
    // last DFG command result.
    dyna_Add("lastDFGCmdResult", LXsTYPE_STRING);
    basic_SetFlags(idx, LXfCMDARG_OPTIONAL | LXfCMDARG_QUERY);
    idx++;
  }
}

// query code.
LxResult FabricCanvasGetResult::Command::cmd_Query(unsigned int index, ILxUnknownID vaQuery)
{
  // init err string.
  std::string err = "command " SERVER_NAME_FabricCanvasGetResult " failed: ";

  // do it.
  CLxUser_ValueArray val_array(vaQuery);
  if (DFGUICmdHandlerDCC::s_lastReturnValue.isEmpty())  val_array.AddString("");
  else                                                  val_array.AddString(DFGUICmdHandlerDCC::s_lastReturnValue.toUtf8().data());

  // done.
  return LXe_OK;
}
