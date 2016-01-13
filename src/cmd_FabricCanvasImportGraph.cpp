#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "cmd_FabricCanvasImportGraph.h"
#include "itm_CanvasIM.h"
#include "itm_CanvasPI.h"
#include "itm_CanvasPIpilot.h"

#include <fstream>
#include <streambuf>

// static tag description interface.
LXtTagInfoDesc FabricCanvasImportGraph::Command::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};

// constructor.
FabricCanvasImportGraph::Command::Command(void)
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
void FabricCanvasImportGraph::Command::cmd_Execute(unsigned flags)
{
  // init err string,
  std::string err = "command " SERVER_NAME_FabricCanvasImportGraph " failed: ";

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
  if (!b) b = CanvasPIpilot::GetBaseInterface(item);
  if (!b)
  { err += "failed to get BaseInterface, item probably has the wrong type";
    feLogError(err);
    return; }

  // get filepath.
  std::string filePath;
  {
    static QString last_fPath;
    QString filter = "DFG Preset (*.canvas)";
    QString fPath = QFileDialog::getOpenFileName(FabricDFGWidget::getPointerAtMainWindow(), "Open DFG Preset", last_fPath, filter, &filter);
    if (fPath.length() == 0)
      return;
    if (fPath.toLower().endsWith(".canvas.canvas"))
      fPath = fPath.left(fPath.length() - std::string(".canvas").length());
    last_fPath = fPath;
    filePath   = fPath.toUtf8().constData();
  }

  // read JSON file.
  std::ifstream t(filePath.c_str(), std::ios::binary);
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
    do
    {
      // set from JSON.
      b->setFromJSON(json.c_str());

      // delete all user channels.
      std::string oErr;
      if (!ModoTools::DeleteAllUserChannels(&item, oErr))
      { err += oErr;
        feLogError(err);
        break; }

      // get graph.
      FabricCore::DFGExec graph = b->getBinding().getExec();
      if (!graph.isValid())
      { feLogError(err + "failed to get a valid graph.");
        break; }

      // re-create all user channels.
      for (unsigned int fi=0;fi<graph.getExecPortCount();fi++)
      {
        // if the port has the wrong type then skip it.
        if (   graph.getExecPortType(fi) != FabricCore::DFGPortType_In
            && graph.getExecPortType(fi) != FabricCore::DFGPortType_Out)
          continue;

        // create a Modo user channel for this port.
        if (!b->CreateModoUserChannelForPort(b->getBinding(), graph.getExecPortName(fi)))
        { feLogError(err + "creating user channel for port \"" + graph.getExecPortName(fi) + "\" failed. Continuing anyway.");
          continue; }
      }

      // if we have an open DFG widget then refresh it.
      FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(b, false);
      if (w)  w->refreshGraph();
    } while (false);
  }
  catch (FabricCore::Exception e)
  {
    feLogError(e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
  }

  // clear the undo stack.
  ModoTools::ClearUndoStack();
}
