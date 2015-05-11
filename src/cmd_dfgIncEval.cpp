#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "cmd_dfgIncEval.h"
#include "itm_dfgModoIM.h"
#include "itm_dfgModoPI.h"
#include "itm_dfgModoPIold.h"

#include <fstream>
#include <streambuf>

// static tag description interface.
LXtTagInfoDesc dfgIncEval::Command::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};

// constructor.
dfgIncEval::Command::Command(void)
{
  // arguments.
  int idx = 0;
  {
    // item's name.
    dyna_Add("item", LXsTYPE_STRING);
    basic_SetFlags(idx, LXfCMDARG_OPTIONAL);
    idx++;

    // item's name.
    dyna_Add("onlyIfZero", LXsTYPE_BOOLEAN);
    basic_SetFlags(idx, LXfCMDARG_OPTIONAL);
    idx++;
  }
}

// execute code.
void dfgIncEval::Command::cmd_Execute(unsigned flags)
{
  // init err string,
  std::string err = "command " SERVER_NAME_dfgIncEval " failed: ";

  // declare and set item from argument.
  CLxUser_Item item;
  if (dyna_IsSet(0))
  {
    // get argument.
    std::string argItemName;
    if (!dyna_String(0, argItemName))
    { err += "failed to read argument";
      feLogError(err);
      return; }

    // get the item.
    if (!ModoTools::GetItem(argItemName, item))
    { err += "the item \"" + argItemName + "\" doesn't exists or cannot be used with this command";
      feLogError(err);
      return; }
  }

  // is item invalid?
  if (!item.test())
  { err += "invalid item";
    feLogError(err);
    return; }

  // get item's BaseInterface.
  // note: we don't really need it, but that way we know we have a valid item.
  BaseInterface *b = NULL;
  if (!b) b = dfgModoIM::GetBaseInterface(item);
  if (!b) b = dfgModoPI::GetBaseInterface(item);
  if (!b) b = dfgModoPIold::GetBaseInterface(item);
  if (!b)
  { err += "failed to get BaseInterface, item probably has the wrong type";
    feLogError(err);
    return;  }

  // get current FabricEval value.
  int eval = 0;
  {
    CLxUser_ChannelRead chanRead;
    if (!chanRead.from(item))
    { err += "failed to create channel reader.";
      feLogError(err);
      return;  }
    eval = chanRead.IValue(item, CHN_NAME_IO_FabricEval);
  }

  // check "onlyIfZero" flag.
  if (dyna_IsSet(1))
  {
    bool onlyIfZero = dyna_Bool(1, false);
    if (onlyIfZero && eval != 0)
      return;
  }

  // increase FabricEval value by 1.
  CLxUser_ChannelWrite chanWriter;
  if (!chanWriter.from(item))
  { err += "failed to create channel writer.";
    feLogError(err);
    return;  }
  if (!chanWriter.Set(item, CHN_NAME_IO_FabricEval, eval + 1))
  { err += "failed to increase eval counter.";
    feLogError(err);
    return;  }
}
