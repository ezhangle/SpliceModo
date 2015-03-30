//
#ifndef SPLICE_MODO_PLUGIN
#define SPLICE_MODO_PLUGIN

// plugin version.
#define FABRICMODO_PLUGIN_VERSION   0.009

// disable some annoying VS warnings.
#pragma warning (disable : 4530)    // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc.
#pragma warning (disable : 4800)    // forcing value to bool 'true' or 'false'.
#pragma warning (disable : 4806)    // unsafe operation: no value of type 'bool' promoted to type ...etc.

// includes.
#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "cmdLogFabricVersion.h"
#include "dfgModoIM.h"

// log system (2/2).
#define LOG_SYSTEM_NAME "Fabric"
class CItemLog : public CLxLogMessage
{
    public:
    CItemLog() : CLxLogMessage(LOG_SYSTEM_NAME) {   }
    const char *GetFormat()     {   return "n.a.";  }
    const char *GetVersion()    {   return "n.a.";  };
    const char *GetCopyright()  {   return "n.a.";  };
};
extern CItemLog gLog;
void feLog(void *userData, const char *s, unsigned int length);
void feLogError(void *userData, const char *s, unsigned int length);

#endif

