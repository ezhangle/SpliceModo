#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "cmd_dfgImportJSON.h"
#include "itm_dfgModoIM.h"
#include "itm_dfgModoPI.h"

#include <fstream>
#include <streambuf>

// static tag description interface.
LXtTagInfoDesc dfgImportJSON::Command::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};

// constructor.
dfgImportJSON::Command::Command(void)
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
void dfgImportJSON::Command::cmd_Execute(unsigned flags)
{
  // init err string,
  std::string err = "command " SERVER_NAME_dfgImportJSON " failed: ";

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
    return; }

  // get filepath.
  std::string filePath;
  {
    static QString last_fPath;
    QString filter = "DFG Preset (*.dfg.json)";
    QString fPath = QFileDialog::getOpenFileName(FabricDFGWidget::getPointerAtMainWindow(), "Open DFG Preset", last_fPath, filter, &filter);
    if (fPath.length() == 0)
      return;
    if (fPath.toLower().endsWith(".dfg.json.dfg.json"))
      fPath = fPath.left(fPath.length() - std::string(".dfg.json").length());
    last_fPath = fPath;
    filePath   = fPath.toUtf8().constData();
  }

  // read JSON file.
  std::ifstream t(filePath, std::ios::binary);
  if (!t.good())
  { err += "unable to open \"" + filePath + "\"";
    feLogError(err);
    return; }
  t.seekg(0, std::ios::end);
  std::string json;
  json.reserve(t.tellg());
  t.seekg(0, std::ios::beg);
  json.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());

  // do it.
  try
  {
    // set from JSON.
    b->setFromJSON(json.c_str());

    // delete all user channels.
    std::string oErr;
    if (!ModoTools::DeleteAllUserChannels(&item, oErr))
    { err += oErr;
      feLogError(err);
      return;    }

    // re-create all user channels.
    FabricServices::DFGWrapper::PortList portlist = b->getGraph()->getPorts();
    for (int fi = 0; fi < portlist.size(); fi++)
    {
      // get pointer at port.
      if (portlist[fi].isNull())
        continue;
      FabricServices::DFGWrapper::PortPtr port = portlist[fi];

      // if the port has the wrong type then skip it.
      if (   port->getPortType() != FabricCore::DFGPortType_In
          && port->getPortType() != FabricCore::DFGPortType_Out)
        continue;

      if (!b->CreateModoUserChannelForPort(port))
      { feLogError(err + "creating user channel for port \"" + port->getName() + "\" failed. Continuing anyway.");
        return; }
    }

    // if we have an open DFG widget then refresh it.
    FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(b, false);
    if (w)  w->refreshGraph();
  }
  catch (FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
  }
}
