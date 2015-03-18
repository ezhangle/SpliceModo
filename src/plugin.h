// includes.
#include "FabricSplice.h"

// more includes.
#include "lx_plugin.hpp"
#include "lx_value.hpp"
#include "lxu_command.hpp"
#include "lxu_log.hpp"
#include "lxlog.h"

// log system.
#define	LOG_SYSTEM_NAME	"Fabric"
class CItemLog : public CLxLogMessage
{
	public:
	CItemLog() : CLxLogMessage(LOG_SYSTEM_NAME)	{	}
	const char *GetFormat()		{	return "n.a.";	}
 	const char *GetVersion()	{	return "n.a.";	};
 	const char *GetCopyright()	{	return "n.a.";	};
};
extern CItemLog gLog;
