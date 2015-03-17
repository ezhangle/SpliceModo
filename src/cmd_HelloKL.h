// includes.
#include "lx_plugin.hpp"
#include "lx_value.hpp"
#include "lxu_command.hpp"
#include "lxu_log.hpp"
#include "lxlog.h"

// constants.
#define	SERVER_NAME		"HelloWorld"

// class.
class cmd_HelloWorld : public CLxBasicCommand
{
	public:

	// static.
	static LXtTagInfoDesc descInfo[];
	static void initialize(void)
	{
		CLxGenericPolymorph *srv =   new CLxPolymorph			<cmd_HelloWorld>;
		srv->AddInterface			(new CLxIfc_Command			<cmd_HelloWorld>);
		srv->AddInterface			(new CLxIfc_Attributes		<cmd_HelloWorld>);
		srv->AddInterface			(new CLxIfc_AttributesUI	<cmd_HelloWorld>);
		srv->AddInterface			(new CLxIfc_StaticDesc		<cmd_HelloWorld>);
		lx::AddServer				(SERVER_NAME, srv);
	};

	// command service.
	int		basic_CmdFlags	(void)						LXx_OVERRIDE	{	return 0;		};
	bool	basic_Enable	(CLxUser_Message &msg)		LXx_OVERRIDE	{	return true;	};
	void	cmd_Execute		(unsigned flags)			LXx_OVERRIDE;
};

// end of file.






