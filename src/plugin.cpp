// includes.
#include "plugin.h"
#include "chanmod_dfgModo.h"
#include "command_feGetPluginVersion.h"

// log system.
CItemLog gLog;
void feLog(void *userData, const char *s, unsigned int length)
{
	gLog.Message(LXe_INFO, "[FABRIC]", s, " ");
	FabricUI::DFG::DFGLogWidget::log(s);
}
void feLogError(void *userData, const char *s, unsigned int length)
{
	gLog.Message(LXe_FAILED, "[FABRIC ERROR]", s, " ");
	std::string t = s;
	t = "Error: " + t;
	FabricUI::DFG::DFGLogWidget::log(t.c_str());
}

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


