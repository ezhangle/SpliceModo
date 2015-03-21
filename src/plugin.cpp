// includes.
#include "plugin.h"
#include "command_feGetFabricVersion.h"
#include "command_feGetPluginVersion.h"

// log system.
CItemLog gLog;
void dfgLog(void *userData, const char *s, unsigned int length) { gLog.Message(LXe_INFO,   "[DFG]",        s, " "); }
void feLog      (const char *s, unsigned int length)            { gLog.Message(LXe_INFO,   "[SPLICE]",     s, " "); }
void feLogError (const char *s, unsigned int length)            { gLog.Message(LXe_FAILED, "[ERROR]",      s, " "); }
void feKLReport (const char *s, unsigned int length)            { gLog.Message(LXe_INFO,   "[KL]",         s, " "); }
void modoLog    (const char *s)                                 { gLog.Message(LXe_INFO,   "[FABRICMODO]", s, " "); }

// plugin initialization.
void initialize()
{
	// Fabric.
	{
		FabricSplice::Initialize();
		FabricSplice::Logging::setLogFunc		(feLog);
		FabricSplice::Logging::setLogErrorFunc	(feLogError);
		FabricSplice::Logging::setKLReportFunc	(feKLReport);
	}

	// MODO.
	{
		command_feGetFabricVersion::initialize();
		command_feGetPluginVersion::initialize();
	}
 }

// plugin clean up.
void cleanup()
{
	// Fabric.
	{
		FabricSplice::Finalize();
	}

	// MODO.
	{
	}
}

// end of file.


