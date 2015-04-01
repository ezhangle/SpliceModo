//
#ifndef SERVER_NAME_cmdPutGraphInFabricJSON
#define SERVER_NAME_cmdPutGraphInFabricJSON "PutGraphInFabricJSON"

namespace cmdPutGraphInFabricJSON
{
    class Command : public CLxBasicCommand
    {
        public:

        // tag description interface.
        static LXtTagInfoDesc descInfo[];

        // initialization.
        static void initialize(void)
        {
            CLxGenericPolymorph *srv =   new CLxPolymorph           <Command>;
            srv->AddInterface           (new CLxIfc_Command         <Command>);
            srv->AddInterface           (new CLxIfc_StaticDesc      <Command>);
            lx:: AddServer              (SERVER_NAME_cmdPutGraphInFabricJSON, srv);
        };

        // command service.
        int     basic_CmdFlags  (void)                      LXx_OVERRIDE    {   return 0;       };
        bool    basic_Enable    (CLxUser_Message &msg)      LXx_OVERRIDE    {   return true;    };
        void    cmd_Execute     (unsigned flags)            LXx_OVERRIDE;
    };
};

#endif

