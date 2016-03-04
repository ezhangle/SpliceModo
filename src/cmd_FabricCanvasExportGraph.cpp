#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "cmd_FabricCanvasExportGraph.h"
#include "itm_CanvasIM.h"
#include "itm_CanvasPI.h"

#include <fstream>
#include <streambuf>

// static tag description interface.
LXtTagInfoDesc FabricCanvasExportGraph::Command::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};

// constructor.
FabricCanvasExportGraph::Command::Command(void)
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
void FabricCanvasExportGraph::Command::cmd_Execute(unsigned flags)
{
  // init err string,
  std::string err = "command " SERVER_NAME_FabricCanvasExportGraph " failed: ";

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
    return;  }

  // get filepath.
  std::string filePath;
  {
    static QString last_fPath;
    QString filter = "DFG Preset (*.canvas)";
    QString fPath = QFileDialog::getSaveFileName(FabricDFGWidget::getPointerAtMainWindow(), "Save DFG Preset", last_fPath, filter, &filter);
    if (fPath.length() == 0)
      return;
    if (fPath.toLower().endsWith(".canvas.canvas"))
      fPath = fPath.left(fPath.length() - std::string(".canvas").length());
    last_fPath = fPath;
    filePath   = fPath.toUtf8().constData();
  }

  // write JSON file.
  std::ofstream t(filePath.c_str(), std::ios::binary);
  if (!t.good())
  { err += "unable to open \"" + filePath + "\"";
    feLogError(err);
    return; }
  try
  {
    std::string json = b->getJSON();
    if (json.c_str())   t.write(json.c_str(), json.length());
    else                t.write("", 0);
  }
  catch (std::exception &e)
  {
    err += "write error for \"" + filePath + "\": ";
    if (e.what())   err += e.what();
    else            err += "";
    feLogError(err);
    return;
  }
}
