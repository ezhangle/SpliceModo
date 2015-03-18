// includes.
#include "lx_plugin.hpp"
#include "lx_value.hpp"
#include "lxu_command.hpp"
#include "lxu_log.hpp"
#include "lxlog.h"

// constants.
#define	SERVER_NAME_HelloGUI		"HelloGUI"

// class.
class cmd_HelloGUI : public CLxBasicCommand
{
	public:

	// static.
	static LXtTagInfoDesc descInfo[];
	static void initialize(void)
	{
		CLxGenericPolymorph *srv =   new CLxPolymorph			<cmd_HelloGUI>;
		srv->AddInterface			(new CLxIfc_Command			<cmd_HelloGUI>);
		srv->AddInterface			(new CLxIfc_Attributes		<cmd_HelloGUI>);
		srv->AddInterface			(new CLxIfc_AttributesUI	<cmd_HelloGUI>);
		srv->AddInterface			(new CLxIfc_StaticDesc		<cmd_HelloGUI>);
		lx::AddServer				(SERVER_NAME_HelloGUI, srv);
	};

	// command service.
	int		basic_CmdFlags	(void)						LXx_OVERRIDE	{	return 0;		};
	bool	basic_Enable	(CLxUser_Message &msg)		LXx_OVERRIDE	{	return true;	};
	void	cmd_Execute		(unsigned flags)			LXx_OVERRIDE;
};

// end of file.






