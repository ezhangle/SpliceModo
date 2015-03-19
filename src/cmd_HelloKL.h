//
#ifndef SERVER_NAME_HelloKL
#define	SERVER_NAME_HelloKL "HelloKL"

// class.
class cmd_HelloKL : public CLxBasicCommand
{
	public:

	// tag description interface.
	static LXtTagInfoDesc descInfo[];

	// initialization.
	static void initialize(void)
	{
		CLxGenericPolymorph *srv =   new CLxPolymorph			<cmd_HelloKL>;
		srv->AddInterface			(new CLxIfc_Command			<cmd_HelloKL>);
		srv->AddInterface			(new CLxIfc_Attributes		<cmd_HelloKL>);
		srv->AddInterface			(new CLxIfc_AttributesUI	<cmd_HelloKL>);
		srv->AddInterface			(new CLxIfc_StaticDesc		<cmd_HelloKL>);
		lx::AddServer				(SERVER_NAME_HelloKL, srv);
	};

	// command service.
	int		basic_CmdFlags	(void)						LXx_OVERRIDE	{	return 0;		};
	bool	basic_Enable	(CLxUser_Message &msg)		LXx_OVERRIDE	{	return true;	};
	void	cmd_Execute		(unsigned flags)			LXx_OVERRIDE;
};

#endif

// end of file.


