//
#ifndef SERVER_NAME_HelloFloat
#define	SERVER_NAME_HelloFloat "HelloFloat"

// forward declarations (because those two reference each other).
class chanmod_HelloFloatPackage;
class chanmod_HelloFloatInstance;

// class (instance).
class chanmod_HelloFloatInstance : public CLxImpl_PackageInstance,
								   public CLxImpl_ChannelModItem
{
	public:

	// tag description interface.
	static LXtTagInfoDesc descInfo[];

	//
	chanmod_HelloFloatPackage  *src_pkg;
	CLxUser_Item				m_item;
	ILxUnknownID				inst_ifc;

        LxResult		 pins_Initialize (ILxUnknownID item, ILxUnknownID super);
        void			 pins_Cleanup (void);
        LxResult		 pins_SynthName (char *buf, unsigned len);

        unsigned int		 cmod_Flags (ILxUnknownID item, unsigned int index);
        LxResult		 cmod_Allocate (
                                        ILxUnknownID cmod,
                                        ILxUnknownID eval,
                                        ILxUnknownID item,
                                        void **ppvData);
        void			 cmod_Cleanup (void *data);
        LxResult		 cmod_Evaluate (ILxUnknownID cmod, ILxUnknownID attr, void *data);
};

// class (instance).
class chanmod_HelloFloatPackage : public CLxImpl_Package
{
    public:
        static LXtTagInfoDesc		 descInfo[];
        CLxPolymorph<chanmod_HelloFloatInstance> chanmod_factory;
        
        chanmod_HelloFloatPackage ();

        LxResult		pkg_SetupChannels (ILxUnknownID addChan);
        LxResult		pkg_TestInterface (const LXtGUID *guid);
        LxResult		pkg_Attach (void **ppvObj);
};



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
	void	cmd_Execute		(unsigned flags)			LXx_OVERRIDE;
};

#endif

// end of file.


