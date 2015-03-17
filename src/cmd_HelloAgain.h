// includes.
#include "lx_plugin.hpp"
#include "lxu_command.hpp"

// defines.
#define	CMD_HELLOAGAIN	"HelloAgain"

// class.
class cmd_HelloAgain : public CLxBasicCommand
{
	public:

	// command service.
	int		basic_CmdFlags	(void)						LXx_OVERRIDE	{	return 0;		};
	bool	basic_Enable	(CLxUser_Message &msg)		LXx_OVERRIDE	{	return true;	};
	void	cmd_Execute		(unsigned flags)			LXx_OVERRIDE;
};

//



