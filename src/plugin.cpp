#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_FabricView.h"
#include "_class_ModoTools.h"
#include "cmd_dfgExportJSON.h"
#include "cmd_dfgImportJSON.h"
#include "cmd_dfgLogVersion.h"
#include "cmd_dfgOpenCanvas.h"
#include "itm_dfgModoIM.h"
#include "itm_dfgModoPI.h"

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
    dfgExportJSON :: Command:: initialize();
    dfgImportJSON :: Command:: initialize();
    dfgLogVersion :: Command:: initialize();
    dfgOpenCanvas :: Command:: initialize();
    dfgModoIM     ::           initialize();
    dfgModoPI     ::           initialize();
    FabricView    ::           initialize();
  }
}

// plugin clean up.
void cleanup()
{
}

