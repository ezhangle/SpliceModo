// includes.
#include "plugin.h"
#include "chanmod_dfgModo.h"
#include "command_feGetPluginVersion.h"

// log system.
CItemLog gLog;
void biLog (void *userData, const char *s, unsigned int length)   { gLog.Message(LXe_INFO,   "[BASEINTERFACE]",   s, " "); }
void feLog                 (const char *s, unsigned int length)   { gLog.Message(LXe_INFO,   "[SPLICE]",          s, " "); }
void feLogError            (const char *s, unsigned int length)   { gLog.Message(LXe_FAILED, "[ERROR]",           s, " "); }
void feKLReport            (const char *s, unsigned int length)   { gLog.Message(LXe_INFO,   "[KL]",              s, " "); }
void modoLog               (const char *s)                        { gLog.Message(LXe_INFO,   "[FABRICMODO]",      s, " "); }

// plugin initialization.
void initialize()
{
	// Fabric.
	{
	}

	// MODO.
	{
		chanmod_dfgModoPackage    ::initialize();
		command_feGetPluginVersion::initialize();
	}
 }

// plugin clean up.
void cleanup()
{
	// Fabric.
	{
	}

	// MODO.
	{
	}
}

// end of file.


