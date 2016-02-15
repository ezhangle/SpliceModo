#include "plugin.h"

#include "_class_DFGUICmdHandlerDCC.h"
#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_FabricView.h"
#include "_class_JSONValue.h"
#include "_class_ModoTools.h"
#include "cmd_FabricCanvasExportGraph.h"
#include "cmd_FabricCanvasImportGraph.h"
#include "cmd_FabricCanvasIncEval.h"
#include "cmd_FabricCanvasLogVersion.h"
#include "cmd_FabricCanvasOpenCanvas.h"
#include "itm_CanvasIM.h"
#include "itm_CanvasPI.h"
#include "itm_CanvasPIpilot.h"

// log system.
class CItemLog : public CLxLogMessage
{
 public:
    CItemLog() : CLxLogMessage(LOG_SYSTEM_NAME) { }
    const char *GetFormat()     { return "n.a."; }
    const char *GetVersion()    { return "n.a."; }
    const char *GetCopyright()  { return "n.a."; }
};
CItemLog gLog;
void feLog(void *userData, const char *s, unsigned int length)
{
  const char *p = (s != NULL ? s : "s == NULL");
  gLog.Message(LXe_INFO, "[FABRIC]", p, " ");
  FabricUI::DFG::DFGLogWidget::log(p);
}
void feLog(void *userData, const std::string &s)
{
  feLog(userData, s.c_str(), s.length());
}
void feLog(const std::string &s)
{
  feLog(NULL, s.c_str(), s.length());
}
void feLogError(void *userData, const char *s, unsigned int length)
{
  const char *p = (s != NULL ? s : "s == NULL");
  gLog.Message(LXe_FAILED, "[FABRIC ERROR]", p, " ");
  std::string t = p;
  t = "Error: " + t;
  FabricUI::DFG::DFGLogWidget::log(t.c_str());
}
void feLogError(void *userData, const std::string &s)
{
  feLogError(userData, s.c_str(), s.length());
}
void feLogError(const std::string &s)
{
  feLogError(NULL, s.c_str(), s.length());
}

// plugin initialization.
void initialize()
{
  // Fabric.
  {
    // set log function pointers.
    BaseInterface::setLogFunc(feLog);
    BaseInterface::setLogErrorFunc(feLogError);
  }

  // Modo.
  {
    FabricCanvasExportGraph :: Command:: initialize();
    FabricCanvasImportGraph :: Command:: initialize();
    FabricCanvasIncEval     :: Command:: initialize();
    FabricCanvasLogVersion  :: Command:: initialize();
    FabricCanvasOpenCanvas  :: Command:: initialize();
    //
    CanvasIM                          :: initialize();
    CanvasPI                          :: initialize();
    CanvasPIpilot                     :: initialize();
    //
    JSONValue                         :: initialize();
    FabricView                        :: initialize();
    //
    FabricCanvasAddBackDrop           :: initialize();
    FabricCanvasAddFunc               :: initialize();
    FabricCanvasAddGet                :: initialize();
    FabricCanvasAddGraph              :: initialize();
    FabricCanvasAddPort               :: initialize();
    FabricCanvasAddSet                :: initialize();
    FabricCanvasAddVar                :: initialize();
    FabricCanvasConnect               :: initialize();
    FabricCanvasCreatePreset          :: initialize();
    FabricCanvasDisconnect            :: initialize();
    FabricCanvasDismissLoadDiags      :: initialize();
    FabricCanvasEditNode              :: initialize();
    FabricCanvasEditPort              :: initialize();
    FabricCanvasExplodeNode           :: initialize();
    FabricCanvasImplodeNodes          :: initialize();
    FabricCanvasInstPreset            :: initialize();
    FabricCanvasMoveNodes             :: initialize();
    FabricCanvasPaste                 :: initialize();
    FabricCanvasRemoveNodes           :: initialize();
    FabricCanvasRemovePort            :: initialize();
    FabricCanvasRenamePort            :: initialize();
    FabricCanvasReorderPorts          :: initialize();
    FabricCanvasResizeBackDrop        :: initialize();
    FabricCanvasSetArgType            :: initialize();
    FabricCanvasSetArgValue           :: initialize();
    FabricCanvasSetCode               :: initialize();
    FabricCanvasSetExtDeps            :: initialize();
    FabricCanvasSetNodeComment        :: initialize();
    FabricCanvasSetPortDefaultValue   :: initialize();
    FabricCanvasSetRefVarPath         :: initialize();
    FabricCanvasSetTitle              :: initialize();
    FabricCanvasSplitFromPreset       :: initialize();
  }
}

// plugin clean up.
void cleanup()
{
}

