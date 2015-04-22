#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "cmd_dfgExportJSON.h"
#include "itm_dfgModoIM.h"
#include "itm_dfgModoPI.h"

#include <fstream>
#include <streambuf>

// static tag description interface.
LXtTagInfoDesc dfgExportJSON::Command::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};

// constructor.
dfgExportJSON::Command::Command(void)
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
void dfgExportJSON::Command::cmd_Execute(unsigned flags)
{
  // init err string,
  std::string err = "command " SERVER_NAME_dfgExportJSON " failed: ";

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
  if (!b) b = dfgModoIM::GetBaseInterface(item);
  if (!b) b = dfgModoPI::GetBaseInterface(item);
  if (!b)
  { err += "failed to get BaseInterface, item probably has the wrong type";
    feLogError(err);
    return;  }

  // get filepath.
  std::string filePath;
  {
    static QString last_fPath;
    QString filter = "DFG Preset (*.dfg.json)";
    QString fPath = QFileDialog::getSaveFileName(FabricDFGWidget::getPointerAtMainWindow(), "Save DFG Preset", last_fPath, filter, &filter);
    if (fPath.length() == 0)
      return;
    if (fPath.toLower().endsWith(".dfg.json.dfg.json"))
      fPath = fPath.left(fPath.length() - std::string(".dfg.json").length());
    last_fPath = fPath;
    filePath   = fPath.toUtf8().constData();
  }

  // write JSON file.
  std::ofstream t(filePath, std::ios::binary);
  if (!t.good())
  { err += "unable to open \"" + filePath + "\"";
    feLogError(err);
    return; }
  try
  {
    std::string json = b->getJSON();
    t.write(json.c_str(), json.length());
  }
  catch (std::exception &e)
  {
    err += "write error for \"" + filePath + "\": " + e.what();
    feLogError(err);
    return;
  }
}
