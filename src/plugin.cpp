// includes.
#include "plugin.h"
#include "chanmod_HelloFloat.h"
#include "cmd_HelloGUI.h"
#include "cmd_HelloKL.h"
#include "cmd_HelloWorld.h"

// log system.
CItemLog gLog;
void feLog		(const char *s, unsigned int length)			{ gLog.Message(LXe_INFO,   "[SPLICE]", s, " "); }
void feLogError	(const char *s, unsigned int length)			{ gLog.Message(LXe_FAILED, "[ERROR]",  s, " "); }
void feKLReport	(const char *s, unsigned int length)			{ gLog.Message(LXe_INFO,   "[KL]",     s, " "); }
void dfgLog(void *userData, const char *s, unsigned int length)	{ gLog.Message(LXe_INFO,   "[DFG]",    s, " "); }

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
		chanmod_HelloFloat	::initialize();
		cmd_HelloGUI		::initialize();
		cmd_HelloKL			::initialize();
		cmd_HelloWorld		::initialize();
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


