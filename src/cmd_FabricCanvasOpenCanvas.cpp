#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "cmd_FabricCanvasOpenCanvas.h"
#include "itm_CanvasIM.h"
#include "itm_CanvasPI.h"

// static tag description interface.
LXtTagInfoDesc FabricCanvasOpenCanvas::Command::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};

// constructor.
FabricCanvasOpenCanvas::Command::Command(void)
{
  // arguments.
  int idx = 0;
  {
    // item's name.
    dyna_Add("item", LXsTYPE_STRING);
    basic_SetFlags(idx, LXfCMDARG_OPTIONAL);
    idx++;
  }
}

// execute code.
void FabricCanvasOpenCanvas::Command::cmd_Execute(unsigned flags)
{
  // init err string,
  std::string err = "command " SERVER_NAME_FabricCanvasOpenCanvas " failed: ";

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
  BaseInterface *b = NULL;
  if (!b) b = CanvasIM::GetBaseInterface(item);
  if (!b) b = CanvasPI::GetBaseInterface(item);
  if (!b)
  { err += "failed to get BaseInterface, item probably has the wrong type";
    feLogError(err);
    return; }

  // get/display DFG widget.
  try
  {
    FabricDFGWidget::getWidgetforBaseInterface(b, true, true);
  }
  catch (FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
  }
}


