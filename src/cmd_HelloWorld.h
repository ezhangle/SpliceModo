//
#ifndef SERVER_NAME_HelloWorld
#define	SERVER_NAME_HelloWorld "HelloWorld"

// class.
class cmd_HelloWorld : public CLxBasicCommand
{
	public:

	// tag description interface.
	static LXtTagInfoDesc descInfo[];

	// initialization.
	static void initialize(void)
	{
		CLxGenericPolymorph *srv =   new CLxPolymorph			<cmd_HelloWorld>;
		srv->AddInterface			(new CLxIfc_Command			<cmd_HelloWorld>);
		srv->AddInterface			(new CLxIfc_Attributes		<cmd_HelloWorld>);
		srv->AddInterface			(new CLxIfc_AttributesUI	<cmd_HelloWorld>);
		srv->AddInterface			(new CLxIfc_StaticDesc		<cmd_HelloWorld>);
		lx::AddServer				(SERVER_NAME_HelloWorld, srv);
	};

	// command service.
	int		basic_CmdFlags	(void)						LXx_OVERRIDE	{	return 0;		};
	bool	basic_Enable	(CLxUser_Message &msg)		LXx_OVERRIDE	{	return true;	};
	void	cmd_Execute		(unsigned flags)			LXx_OVERRIDE;
};

#endif

// end of file.


