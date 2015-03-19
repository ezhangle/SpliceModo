//
#ifndef SERVER_NAME_HelloFloat
#define	SERVER_NAME_HelloFloat "HelloFloat"

// class.
class chanmod_HelloFloat : public CLxBasicCommand
{
	public:

	// tag description interface.
	static LXtTagInfoDesc descInfo[];

	// initialization.
	static void initialize(void)
	{
		CLxGenericPolymorph *srv =   new CLxPolymorph			<chanmod_HelloFloat>;
		srv->AddInterface			(new CLxIfc_Command			<chanmod_HelloFloat>);
		srv->AddInterface			(new CLxIfc_Attributes		<chanmod_HelloFloat>);
		srv->AddInterface			(new CLxIfc_AttributesUI	<chanmod_HelloFloat>);
		srv->AddInterface			(new CLxIfc_StaticDesc		<chanmod_HelloFloat>);
		lx::AddServer				(SERVER_NAME_HelloFloat, srv);
	};

	// command service.
	int		basic_CmdFlags	(void)						LXx_OVERRIDE	{	return 0;		};
	bool	basic_Enable	(CLxUser_Message &msg)		LXx_OVERRIDE	{	return true;	};
	void	chanmod_Execute		(unsigned flags)			LXx_OVERRIDE;
};

#endif

// end of file.


