//
#ifndef SERVER_NAME_command_feGetFabricVersion
#define	SERVER_NAME_command_feGetFabricVersion "feGetFabricVersion"

// class.
class command_feGetFabricVersion : public CLxBasicCommand
{
	public:

	// tag description interface.
	static LXtTagInfoDesc descInfo[];

	// initialization.
	static void initialize(void)
	{
		CLxGenericPolymorph *srv =   new CLxPolymorph			<command_feGetFabricVersion>;
		srv->AddInterface			(new CLxIfc_Command			<command_feGetFabricVersion>);
		srv->AddInterface			(new CLxIfc_Attributes		<command_feGetFabricVersion>);
		srv->AddInterface			(new CLxIfc_AttributesUI	<command_feGetFabricVersion>);
		srv->AddInterface			(new CLxIfc_StaticDesc		<command_feGetFabricVersion>);
		lx:: AddServer				(SERVER_NAME_command_feGetFabricVersion, srv);
	};

	// command service.
	int		basic_CmdFlags	(void)						LXx_OVERRIDE	{	return 0;		};
	bool	basic_Enable	(CLxUser_Message &msg)		LXx_OVERRIDE	{	return true;	};
	void	cmd_Execute		(unsigned flags)			LXx_OVERRIDE;
};

#endif

// end of file.


