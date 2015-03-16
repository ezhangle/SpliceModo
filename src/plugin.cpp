/* ---------------------------------------------------------------

	A simple "hello world!" command.

	Usage:	execute the command "hello.world" and check the
			output in the event log.

--------------------------------------------------------------- */

// includes.
#include <lx_log.hpp>
#include "lx_plugin.hpp"
#include "lx_value.hpp"
#include "lxu_command.hpp"

// defines. 
#define COMMAND_NAME	"hello.world"		// the name of the command.

// forward declarations.
class cmd_helloWorld;

// init.
void initialize()
{
	CLxGenericPolymorph *srv;
 
	srv = new CLxPolymorph							<cmd_helloWorld>;
	srv->AddInterface	(new CLxIfc_Command         <cmd_helloWorld>);
	srv->AddInterface	(new CLxIfc_Attributes      <cmd_helloWorld>);
	srv->AddInterface	(new CLxIfc_AttributesUI    <cmd_helloWorld>);
	lx::AddServer		(COMMAND_NAME, srv);
}

// clean up.
void cleanup()
{
}

// the command class.
class cmd_helloWorld : public CLxBasicCommand
{
	public:
	CLxUser_CommandService		cmd_srv;

	// command service.
	int		basic_CmdFlags		(void)						LXx_OVERRIDE;
	bool	basic_Enable		(CLxUser_Message &msg)		LXx_OVERRIDE;
	void	cmd_Execute			(unsigned flags)			LXx_OVERRIDE;
};
 
//
int cmd_helloWorld::basic_CmdFlags(void)
{
	/* NOTE:
		This is a pure virtual function, so we need to implement
		it even though we are simply going to return 0.
	*/
	return 0;
}
 
//
bool cmd_helloWorld::basic_Enable(CLxUser_Message &msg)
{
	/* NOTE:
		Return true to enable the command, else false.
		This can be used to check things before actually executing the command,
		for example one could return false if some input arguments are incorrect.
	*/
	return true;
}
 
//
void cmd_helloWorld::cmd_Execute(unsigned flags)
{
	// NOTE: uncommenting the following two lines result in a crash when executingthe command.

	//CLxUser_Log log;
 	//log.Info("Hello World!");
}
 
// end of file



