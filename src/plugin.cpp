// includes.
#include "plugin.h"
#include "cmd_HelloKL.h"
#include "cmd_HelloWorld.h"

// log stuff.
CItemLog gLog;
void feLog		(const char *s, unsigned int length)	{ gLog.Message(LXe_INFO,   "[SPLICE]", s, " "); }
void feLogError	(const char *s, unsigned int length)	{ gLog.Message(LXe_FAILED, "[ERROR]",  s, " "); }
void feKLReport	(const char *s, unsigned int length)	{ gLog.Message(LXe_INFO,   "[KL]",     s, " "); }

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
		cmd_HelloWorld	::initialize();
		cmd_HelloKL		::initialize();
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


