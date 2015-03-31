#include "plugin.h"

#include "Windows.h"

#define SERVER_NAME_dfgModoIM       "dfgModoIM"

#define CHN_NAME_IO_FabricActive    "FabricActive"  // io: enable/disable execution of DFG for this item.
#define CHN_NAME_IO_FabricJSON      "FabricJSON"    // io: string for BaseInterface::getJSON() and BaseInterface::setFromJSON().

static CLxItemType gItemType_dfgModoIM (SERVER_NAME_dfgModoIM);

BaseInterface *quickhack_baseInterface = NULL;

namespace dfgModoIM
{
    /*
     *  Implement the Package and Instance. The instance doesn't have to do anything,
     *  but the package adds the standard set of channels and marks certain of those
     *  channels as schematic inputs/outputs. We also implement a SceneItemListener,
     *  allowing us to invalidate the modifier when new channels are added.
     */

    // user data.
    struct _userdata
    {
        BaseInterface *baseInterface; // Fabric Engine base interface.

        _userdata()
        {
            gLog.Message(LXe_INFO, "[dfgModoIM]", "init user data  / create quick hack base instance", " ");
            quickhack_baseInterface = new BaseInterface();

            gLog.Message(LXe_INFO, "[dfgModoIM]", "init user data  / create base instance", " ");
            memset(this, NULL, sizeof(*this));      // zero out.
            baseInterface = new BaseInterface();    // create base instance.
        };

        ~_userdata()
        {
            if (quickhack_baseInterface)
            {
                gLog.Message(LXe_INFO, "[dfgModoIM]", "clear user data / destroy quick hack base instance", " ");
                // delete widget and base interface.
                FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(quickhack_baseInterface, false);
                if (w) delete w;
                delete quickhack_baseInterface;
            }

            if (baseInterface)
            {
                gLog.Message(LXe_INFO, "[dfgModoIM]", "clear user data / destroy base instance", " ");
                // delete widget and base interface.
                FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(baseInterface, false);
                if (w) delete w;
                delete baseInterface;
            }
        };
    };

    class Instance : public CLxImpl_PackageInstance
    {
        public:
            static void initialize()
            {
                CLxGenericPolymorph *srv = NULL;
                srv = new CLxPolymorph                              <Instance>;
                srv->AddInterface       (new CLxIfc_PackageInstance <Instance>);
                lx::AddSpawner          (SERVER_NAME_dfgModoIM ".inst", srv);
            }

            _userdata *m_userdata;

            Instance()
            {
                m_userdata = new _userdata;
            };

            ~Instance()
            {
                delete m_userdata;
            };
    };

    class Package : public CLxImpl_Package, public CLxImpl_ChannelUI, public CLxImpl_SceneItemListener
    {
        public:
        static void initialize()
        {
            CLxGenericPolymorph *srv = NULL;
            srv = new CLxPolymorph                                  <Package>;
            srv->AddInterface        (new CLxIfc_Package            <Package>);
            srv->AddInterface        (new CLxIfc_StaticDesc         <Package>);
            srv->AddInterface        (new CLxIfc_SceneItemListener  <Package>);
            srv->AddInterface        (new CLxIfc_ChannelUI          <Package>);
            lx::AddServer            (SERVER_NAME_dfgModoIM, srv);
        }

        Package() : m_inst_spawn (SERVER_NAME_dfgModoIM ".inst") {}
    
        LxResult    pkg_SetupChannels   (ILxUnknownID addChan_obj)                          LXx_OVERRIDE;
        LxResult    pkg_Attach          (void **ppvObj)                                     LXx_OVERRIDE;
        LxResult    pkg_TestInterface   (const LXtGUID *guid)                               LXx_OVERRIDE;
        
        LxResult    cui_UIHints         (const char *channelName, ILxUnknownID hints_obj)   LXx_OVERRIDE;
    
        void        sil_ItemAddChannel  (ILxUnknownID item_obj)                             LXx_OVERRIDE;

        static LXtTagInfoDesc descInfo[];
    
        private:
            CLxSpawner <Instance> m_inst_spawn;
    };

    LxResult Package::pkg_SetupChannels(ILxUnknownID addChan_obj)
    {
        /*
         *  Add some basic built in channels.
         */

        CLxUser_AddChannel  add_chan(addChan_obj);
        LxResult            result = LXe_FAILED;

        if (add_chan.test())
        {
            add_chan.NewChannel(CHN_NAME_IO_FabricActive, LXsTYPE_BOOLEAN);
            add_chan.SetDefault(1, 1);

            add_chan.NewChannel(CHN_NAME_IO_FabricJSON, LXsTYPE_STRING);
            add_chan.SetStorage(LXsTYPE_STRING);

            result = LXe_OK;
        }

        return result;
    }

    LxResult Package::pkg_Attach(void **ppvObj)
    {
        /*
         *  Allocate an instance of the package instance.
         */

        m_inst_spawn.Alloc(ppvObj);
    
        return (ppvObj[0] ? LXe_OK : LXe_FAILED);
    }

    LxResult Package::pkg_TestInterface(const LXtGUID *guid)
    {
        /*
         *  This is called for the various interfaces this package could
         *  potentially support, it should return a result code to indicate
         *  if it implements the specified guid.
         */

        return m_inst_spawn.TestInterfaceRC(guid);
    }

    LxResult Package::cui_UIHints(const char *channelName, ILxUnknownID hints_obj)
    {
        /*
         *  Here we set some hints for the built in channels. These allow channels
         *  to be displayed as either inputs or outputs in the schematic. 
         */

        CLxUser_UIHints hints(hints_obj);
        LxResult        result = LXe_FAILED;
    
        if (hints.test())
        {
            if (strcmp(channelName, "draw"))
            {
                if (   !strcmp(channelName, CHN_NAME_IO_FabricActive)
                    || !strcmp(channelName, CHN_NAME_IO_FabricJSON)
                   )
                {
                    result = hints.ChannelFlags (/*LXfUIHINTCHAN_INPUT_ONLY | */LXfUIHINTCHAN_SUGGESTED);
                }
                else
                {
                    result = hints.ChannelFlags (/*LXfUIHINTCHAN_OUTPUT_ONLY | */LXfUIHINTCHAN_SUGGESTED);
                }
            }
            result = LXe_OK;
        }
    
        return result;
    }

    void Package::sil_ItemAddChannel(ILxUnknownID item_obj)
    {
        /*
         *  When user channels are added to our item type, this function will be
         *  called. We use it to invalidate our modifier so that it's reallocated.
         *  We don't need to worry about channels being removed, as the evaluation
         *  system will automatically invalidate the modifier when channels it
         *  writes are removed.
         */
    
        CLxUser_Item    item(item_obj);
        CLxUser_Scene   scene;
    
        if (item.test() && item.IsA(gItemType_dfgModoIM.Type ()))
        {
            if (item.GetContext(scene))
                scene.EvalModInvalidate(SERVER_NAME_dfgModoIM ".mod");
        }
    }

    LXtTagInfoDesc Package::descInfo[] =
    {
        { LXsPKG_SUPERTYPE, LXsITYPE_ITEMMODIFY },
        { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
        { 0 }
    };

    /*
     *  Implement the Modifier Element and Server. This reads the input channels as
     *  read only channels and output channels as write only channels.
     */

    struct ChannelDef
    {
        int  chan_index;
        int  eval_index;
        std::string chan_name;
    
        ChannelDef () : chan_index (-1), eval_index (-1), chan_name("") {}
    };

    class Element : public CLxItemModifierElement
    {
        public:
            Element                 (CLxUser_Evaluation &eval, ILxUnknownID item_obj);
            bool         Test           (ILxUnknownID item_obj)                 LXx_OVERRIDE;
            void         Eval           (CLxUser_Evaluation &eval, CLxUser_Attributes &attr)    LXx_OVERRIDE;
    
        private:
            void         userChannels_collect   (CLxUser_Item &item, std::vector <ChannelDef> &userChannels);
    
            int          m_chan_index;
            std::vector <ChannelDef> m_user_channels;
    };

    class Modifier : public CLxItemModifierServer
    {
        public:
            static void initialize()
            {
                CLxExport_ItemModifierServer <Modifier> (SERVER_NAME_dfgModoIM ".mod");
            }
    
            const char  *ItemType       ()                          LXx_OVERRIDE;
    
            CLxItemModifierElement *Alloc       (CLxUser_Evaluation &eval, ILxUnknownID item_obj)   LXx_OVERRIDE;
    };

    Element::Element (CLxUser_Evaluation &eval, ILxUnknownID item_obj)
    {
        {
            //
            (*quickhack_baseInterface).m_item_obj_dfgModoIM = item_obj;

             // w.i.p.
            {
                FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(quickhack_baseInterface);
                if (w && !(*w).isVisible())
                    (*w).show();
            }
        }

        /*
         *  In the constructor, we want to add the input and output channels
         *  required for this modifier. The inputs are hardcoded, but for the
         *  outputs, we want to scan through all the user channels on the current
         *  item and add those. We cache the user channels, so we can easily check
         *  when they've changed.
         */

        CLxUser_Item item(item_obj);
        if (!item.test())
            return;

        /*
         *  The first channels we want to add are the standard input channels.
         */

        m_chan_index =  eval.AddChan (item, CHN_NAME_IO_FabricActive, LXfECHAN_READ);
                        eval.AddChan (item, CHN_NAME_IO_FabricJSON, LXfECHAN_READ);

        /*
         *  Next, we want to grab all of the user channels on the item and add
         *  them as output channels to the modifier. We cache the list of
         *  user channels for easy access from our modifier.
         */
    
        userChannels_collect (item, m_user_channels);
    
        for (unsigned i = 0; i < m_user_channels.size (); i++)
        {
            ChannelDef      *channel = &m_user_channels[i];
        
            channel->eval_index = eval.AddChan(item, channel->chan_index, LXfECHAN_READ | LXfECHAN_WRITE);
        }
    }

    bool Element::Test(ILxUnknownID item_obj)
    {
        /*
         *  When the list of user channels for a particular item changes, the
         *  modifier will be invalidated. This function will be called to check
         *  if the modifier we allocated previously matches what we'd allocate
         *  if the Alloc function was called now. We return true if it does. In
         *  our case, we check if the current list of user channels for the
         *  specified item matches what we cached when we allocated the modifier.
         */
    
        CLxUser_Item         item (item_obj);
        std::vector <ChannelDef> user_channels;

        if (item.test())
        {
            userChannels_collect (item, user_channels);
        
            return user_channels.size () == m_user_channels.size ();
        }
    
        return false;
    }

    void Element::Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)
    {
         // w.i.p.
        {
            FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(quickhack_baseInterface);
            if (w && !(*w).isVisible())
                (*w).show();
        }

         // debug: output amount of channels and channel names in attr.
        {
            char s[256];
            sprintf(s, "attr.Count() = %ld  (m_chan_index = %ld)", attr.Count(), m_chan_index);
            gLog.Message(LXe_INFO, "[DBG]", s, " ");
            for (int i=0;i<m_user_channels.size();i++)
            {
                ChannelDef &c = m_user_channels[i];
                sprintf(s, "   name = %s   (chan_index = %ld, eval_index = %ld)", c.chan_name.c_str(), c.chan_index, c.eval_index);
                gLog.Message(LXe_INFO, "[DBG]", s, " ");
            }
        }

       // nothing to do?
        if (!eval || !attr)
            return;

        // ref at DFG stuff.
        FabricServices::DFGWrapper::Binding         &binding = *(*quickhack_baseInterface).getBinding();
        FabricServices::DFGWrapper::GraphExecutable &graph   = binding.getGraph();

        // read the fixed input channels and return early if the FabricActive flag is disabled.
        {
            int FabricActive = false;
            unsigned int temp_chan_index = m_chan_index;
            attr.GetInt(temp_chan_index++, &FabricActive);
            temp_chan_index++;  // note: we don't need the FabricJSON string here, so no need to fetch it.
            if (!FabricActive)
                return;
        }

        // Fabric Engine (step 1): loop through all the DFG's input ports and set
        //                         their values from the matching Modo user channels.
        {
            std::vector <FabricServices::DFGWrapper::Port> ports = graph.getPorts();
            for (int fi=0;fi<ports.size();fi++)
            {
                FabricServices::DFGWrapper::Port &port = ports[fi];

                gLog.Message(LXe_INFO, "[DBG]", port.getName().c_str(), " ");

                int mi = -1;
                for (int i=0;i<m_user_channels.size();i++)
                {
                    ChannelDef &c = m_user_channels[i];
                    if (port.getName() == c.chan_name)
                    {
                        mi = c.eval_index;
                    }
                }


                if (mi < 0)
                    gLog.Message(LXe_INFO, "[DBG]", "no matchind Modo port was found", " ");
                else
                {
                    char s[256];
                    sprintf(s, "found matchind Modo port (eval index = %ld)", mi);
                    gLog.Message(LXe_INFO, "[DBG]", s, " ");
                }
            }
        }

        // Fabric Engine (step 2): execute the DFG.
        {
            try
            {
                binding.execute();
            }
            catch(FabricCore::Exception e)
            {
                feLogError(NULL, e.getDesc_cstr(), e.getDescLength());
            }
        }

        // Fabric Engine (step 3): loop through all the DFG's output ports and set
        //                         the values of the matching Modo user channels.
        {
            // will be implemented tuesday (april 31.) morning.
        }

        // done.
        return;
    }

    void Element::userChannels_collect (CLxUser_Item &item, std::vector <ChannelDef> &userChannels)
    {
        /*
         *  This function collects all of the user channels on the specified item
         *  and adds them to the provided vector of channel definitions. We loop
         *  through all channels on the item and test their package. If they have
         *  no package, then it's a user channel and it's added to the vector.
         *  We also check if the channel type is a divider, if it is, we skip it.
         */
    
        unsigned count = 0;
    
        userChannels.clear();
    
        if (!item.test())
            return;
    
        item.ChannelCount(&count);
    
        for (unsigned i=0;i<count;i++)
        {
            const char *package      = NULL;
            const char *channel_type = NULL;
    
            // does it have a package, i.e. not a user channel?
            if (LXx_OK (item.ChannelPackage (i, &package)) || package)
                continue;
        
            if (LXx_OK (item.ChannelEvalType (i, &channel_type) && channel_type))
            {
                // no type, i.e. is this a divider?
                if (!strcmp(channel_type, LXsTYPE_NONE))
                    continue;

                //
                ChannelDef channel;

                channel.chan_index = i;
                channel.eval_index = -1;

                const char *name = NULL;
                item.ChannelName(i, &name);
                channel.chan_name  = name;

                userChannels.push_back (channel);
            }
        }
    }

    const char * Modifier::ItemType ()
    {
        /*
         *  The modifier should only associate itself with this item type.
         */

        return SERVER_NAME_dfgModoIM;
    }

    CLxItemModifierElement * Modifier::Alloc (CLxUser_Evaluation &eval, ILxUnknownID item)
    {
        /*
         *  Allocate and return the modifier element.
         */
        return new Element (eval, item);
    }

    // used in the plugin's initialize() function (see plugin.cpp).
    void initialize()
    {
        Instance::initialize();
        Package ::initialize();
        Modifier::initialize();
    }
};



