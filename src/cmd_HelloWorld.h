// includes.
#include <lx_log.hpp>
#include "lx_plugin.hpp"
#include "lx_value.hpp"
#include "lxu_command.hpp"
#include "lxu_log.hpp"

// defines.
#define	CMD_HelloWorld	"HelloWorld"

// class.
class cmd_HelloWorld : public CLxBasicCommand
{
	public:

	cmd_HelloWorld(void)	{	};
	~cmd_HelloWorld()		{	};

	// command service.
	int		basic_CmdFlags		(void)						LXx_OVERRIDE	{	return 0;		};
	bool	basic_Enable		(CLxUser_Message &msg)		LXx_OVERRIDE	{	return true;	};
	void	cmd_Execute			(unsigned flags)			LXx_OVERRIDE;
};

//



