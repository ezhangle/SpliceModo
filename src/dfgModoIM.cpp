#include "plugin.h"

#include "Windows.h"

#define SERVER_NAME_dfgModoIM       "dfgModoIM"

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
                m_item      = NULL;
                m_userdata  = new _userdata;
            };

            ~Instance()
            {
                delete m_userdata;
            };

            LxResult    pins_Initialize(ILxUnknownID item, ILxUnknownID super)  LXx_OVERRIDE;
            LxResult    pins_AfterLoad(void)                                    LXx_OVERRIDE;

        private:
            ILxUnknownID m_item;
    };

    LxResult Instance::pins_Initialize(ILxUnknownID item, ILxUnknownID super)
    {
        m_item = item;

        return LXe_OK;
    }

    LxResult Instance::pins_AfterLoad(void)
    {
        // init err string,
        std::string err = "pins_AfterLoad() failed: ";

        // check pointer and create ref at BaseInterface.
        if (!quickhack_baseInterface)
        {   err += "pointer == NULL";
            feLogError(0, err.c_str(), err.length());
            return LXe_OK;  }
        BaseInterface &b = *quickhack_baseInterface;

        // create item.
        CLxUser_Item item(m_item);
        if (!item.test())
        {   err += "item(m_item) failed";
            feLogError(0, err.c_str(), err.length());
            return LXe_OK;  }

        // log.
        std::string itemName;
        item.GetUniqueName(itemName);
        std::string info;
        info = "item \"" + itemName + "\": set Fabric base interface from item's JSON string.";
        feLog(0, info.c_str(), info.length());

        // get content of channel CHN_NAME_IO_FabricJSON.
        CLxUser_ChannelRead chanRead;
        chanRead.from(item);
        if (!chanRead.from(item))
        {   err += "couldn't create channel reader.";
            feLogError(0, err.c_str(), err.length());
            return LXe_OK;  }
        std::string json;
        if (!chanRead.GetString(item, CHN_NAME_IO_FabricJSON, json))
        {   err += "failed to read channel \"" CHN_NAME_IO_FabricJSON "\"";
            feLogError(0, err.c_str(), err.length());
            return LXe_OK;  }

        // do it.
        b.setFromJSON(json);

        // done.
        return LXe_OK;
    }

    class Package : public CLxImpl_Package,
                    public CLxImpl_ChannelUI,
                    public CLxImpl_SceneItemListener
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
            add_chan.SetInternal();

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
                    result = hints.ChannelFlags (0);    // by default we don't display the fixed channels in the schematic view.
                }
                else
                {
                    if      ((*quickhack_baseInterface).HasInputPort(channelName))  result = hints.ChannelFlags(LXfUIHINTCHAN_INPUT_ONLY  | LXfUIHINTCHAN_SUGGESTED);
                    else if ((*quickhack_baseInterface).HasOutputPort(channelName)) result = hints.ChannelFlags(LXfUIHINTCHAN_OUTPUT_ONLY | LXfUIHINTCHAN_SUGGESTED);
                    else                                                            result = hints.ChannelFlags(0);
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
            Element(CLxUser_Evaluation &eval, ILxUnknownID item_obj);
            bool         Test(ILxUnknownID item_obj)                                LXx_OVERRIDE;
            void         Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)   LXx_OVERRIDE;
    
        private:
            int                      m_eval_index_FabricActive;
            int                      m_eval_index_FabricJSON;
            std::vector <ChannelDef> m_usrChannels;
            void        usrChannelsCollect    (CLxUser_Item &item, std::vector <ChannelDef> &io_usrChannels);
            ChannelDef *usrChannelsGetFromName(std::string channelName, std::vector <ChannelDef> &usrChannels);
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

    Element::Element(CLxUser_Evaluation &eval, ILxUnknownID item_obj)
    {
        {
            //
            (*quickhack_baseInterface).m_item_obj_dfgModoIM = item_obj;
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

        // add the fixed input channels to eval.
        m_eval_index_FabricActive = eval.AddChan(item, CHN_NAME_IO_FabricActive, LXfECHAN_READ);
        m_eval_index_FabricJSON   = eval.AddChan(item, CHN_NAME_IO_FabricJSON,   LXfECHAN_READ);

        // collect all the user channels and add them to eval.
        usrChannelsCollect(item, m_usrChannels);
        for (unsigned i=0;i<m_usrChannels.size();i++)
        {
            ChannelDef &c = m_usrChannels[i];

            unsigned int type;
            if      ((*quickhack_baseInterface).HasInputPort (c.chan_name.c_str()))     type = LXfECHAN_READ;
            else if ((*quickhack_baseInterface).HasOutputPort(c.chan_name.c_str()))     type =                 LXfECHAN_WRITE;
            else                                                                        type = LXfECHAN_READ | LXfECHAN_WRITE;

            c.eval_index = eval.AddChan(item, c.chan_index, type);

            // debug.
            {
                std::string s = "[DBG] eval.AddChan(\"" + c.chan_name + "\")";
                feLog(NULL, s);
            }
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
    
        CLxUser_Item             item(item_obj);
        std::vector <ChannelDef> tmp;

        if (item.test())
        {
            usrChannelsCollect (item, tmp);
        
            return tmp.size() == m_usrChannels.size();
        }
    
        return false;
    }

    void Element::Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)
    {
       // nothing to do?
        if (!eval || !attr)
            return;

        // refs at DFG wrapper members.
        FabricCore::Client                          &client  = *(*quickhack_baseInterface).getClient();
        FabricServices::DFGWrapper::Binding         &binding = *(*quickhack_baseInterface).getBinding();
        FabricServices::DFGWrapper::GraphExecutable &graph   = binding.getGraph();

        // read the fixed input channels and return early if the FabricActive flag is disabled.
        int FabricActive = false;
        attr.GetInt(m_eval_index_FabricActive, &FabricActive);
        if (!FabricActive)
            return;

        // Fabric Engine (step 1): loop through all the DFG's input ports and set
        //                         their values from the matching Modo user channels.
        {
            try
            {
                char        serr[256];
                std::string err = "";
                std::vector <FabricServices::DFGWrapper::Port> ports = graph.getPorts();
                for (int fi=0;fi<ports.size();fi++)
                {
                    // ref at port.
                    FabricServices::DFGWrapper::Port &port = ports[fi];

                    // if the port has the wrong type then skip it.
                    if (port.getPortType() != FabricCore::DFGPortType_In)
                        continue;

                    // get pointer at matching channel definition.
                    ChannelDef *cd = usrChannelsGetFromName(port.getName(), m_usrChannels);
                    if (!cd || (*cd).eval_index < 0)
                    {   err = "unable to find a user channel that matches the port \"" + port.getName() + "\"";
                        break;  }

                    // "DFG port value = item user channel".
                    int retGet = 0;
                    if      (   port.getDataType() == "Boolean")    {
                                                                        bool val = false;
                                                                        retGet = ModoTools::GetChannelValueAsBoolean(attr, (*cd).eval_index, val);
                                                                        if (retGet == 0)    BaseInterface::SetValueOfPortBoolean(client, binding, port, val);
                                                                    }
                    else if (   port.getDataType() == "SInt8"
                             || port.getDataType() == "SInt16"
                             || port.getDataType() == "SInt32"
                             || port.getDataType() == "SInt64" )    {
                                                                        int val = 0;
                                                                        retGet = ModoTools::GetChannelValueAsInteger(attr, (*cd).eval_index, val);
                                                                        if (retGet == 0)    BaseInterface::SetValueOfPortSInt(client, binding, port, val);
                                                                    }
                    else if (   port.getDataType() == "UInt8"
                             || port.getDataType() == "UInt16"
                             || port.getDataType() == "UInt32"
                             || port.getDataType() == "UInt64" )    {
                                                                        unsigned int val = 0;
                                                                        retGet = ModoTools::GetChannelValueAsInteger(attr, (*cd).eval_index, *(int *)val);
                                                                        if (retGet == 0)    BaseInterface::SetValueOfPortUInt(client, binding, port, val);
                                                                    }
                    else if (   port.getDataType() == "Float32"
                             || port.getDataType() == "Float64" )   {
                                                                        double val = 0;
                                                                        retGet = ModoTools::GetChannelValueAsFloat(attr, (*cd).eval_index, val);
                                                                        if (retGet == 0)    BaseInterface::SetValueOfPortFloat(client, binding, port, val);
                                                                    }
                    else if (   port.getDataType() == "String")     {
                                                                        std::string val = "";
                                                                        retGet = ModoTools::GetChannelValueAsString(attr, (*cd).eval_index, val);
                                                                        if (retGet == 0)    BaseInterface::SetValueOfPortString(client, binding, port, val);
                                                                    }
                    else if (   port.getDataType() == "Quat")       {
                                                                        std::vector <double> val;
                                                                        retGet = ModoTools::GetChannelValueAsQuaternion(attr, (*cd).eval_index, val);
                                                                        if (retGet == 0)    BaseInterface::SetValueOfPortQuat(client, binding, port, val);
                                                                    }
                    else if (   port.getDataType() == "Mat44")      {
                                                                        std::vector <double> val;
                                                                        retGet = ModoTools::GetChannelValueAsMatrix44(attr, (*cd).eval_index, val);
                                                                        if (retGet == 0)    BaseInterface::SetValueOfPortMat44(client, binding, port, val);
                                                                    }
                    else
                    {
                        err = "the port \"" + port.getName() + "\" has the unsupported data type \"" + port.getDataType() + "\"";
                        break;
                    }

                    // error getting value from user channel?
                    if (retGet != 0)
                    {
                        sprintf(serr, "%ld", retGet);
                        err = "failed to get value from user channel \"" + port.getName() + "\" (returned " + serr + ")";
                        break;
                    }
                }

                // error?
                if (err != "")
                {
                    feLogError(NULL, err.c_str(), err.length());
                    return;
                }
            }
            catch(FabricCore::Exception e)
            {
                feLogError(NULL, e.getDesc_cstr(), e.getDescLength());
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
            try
            {
                char        serr[256];
                std::string err = "";
                std::vector <FabricServices::DFGWrapper::Port> ports = graph.getPorts();
                for (int fi=0;fi<ports.size();fi++)
                {
                    // ref at port.
                    FabricServices::DFGWrapper::Port &port = ports[fi];

                    // if the port has the wrong type then skip it.
                    if (port.getPortType() != FabricCore::DFGPortType_Out)
                        continue;

                    // get pointer at matching channel definition.
                    std::string name = port.getName();
                    ChannelDef *cd = usrChannelsGetFromName(name, m_usrChannels);
                    if (!cd || (*cd).eval_index < 0)
                    {   err = "unable to find a user channel that matches the port \"" + name + "\"";
                        break;  }

                    // "item user channel = DFG port value".
                    int dataType = attr.Type((*cd).eval_index);
                    const char *typeName = NULL;
                    if (!LXx_OK(attr.TypeName((*cd).eval_index, &typeName)))
                        typeName = NULL;
                    FabricCore::RTVal rtval;
                    int retGet = 0;
                    int retSet = LXe_OK;
                    if      (dataType == LXi_TYPE_INTEGER)
                    {
                        int val;
                        retGet = BaseInterface::GetPortValueInteger(port, val);
                        if (retGet == 0)
                            retSet = attr.SetInt((*cd).eval_index, val);
                    }
                    else if (dataType == LXi_TYPE_FLOAT)
                    {
                        double val;
                        retGet = BaseInterface::GetPortValueFloat(port, val);
                        if (retGet == 0)
                            retSet = attr.SetFlt((*cd).eval_index, val);
                    }
                    else if (dataType == LXi_TYPE_STRING)
                    {
                        std::string val;
                        retGet = BaseInterface::GetPortValueString(port, val);
                        if (retGet == 0)
                            retSet = attr.SetString((*cd).eval_index, val.c_str());
                    }
                    else if (dataType == LXi_TYPE_OBJECT && typeName && !strcmp (typeName, LXsTYPE_QUATERNION))
                    {
                        std::vector <double> val;
                        retGet = BaseInterface::GetPortValueQuat(port, val);
                        if (retGet == 0 && val.size() == 4)
                        {
                            CLxUser_Quaternion usrQuaternion;
                            LXtQuaternion      q;
                            if (!attr.ObjectRW((*cd).eval_index, usrQuaternion) || !usrQuaternion.test())
                            {   err = "the function ObjectRW() failed for the user channel  \"" + name + "\"";
                                break;  }
                            for (int i=0;i<4;i++)   q[i] = val[i];
                            usrQuaternion.SetQuaternion(q);
                        }
                    }
                    else if (dataType == LXi_TYPE_OBJECT && typeName && !strcmp (typeName, LXsTYPE_MATRIX4))
                    {
                        std::vector <double> val;
                        retGet = BaseInterface::GetPortValueMat44(port, val);
                        if (retGet == 0 && val.size() == 16)
                        {
                            CLxUser_Matrix usrMatrix;
                            LXtMatrix4     m44;
                            if (!attr.ObjectRW((*cd).eval_index, usrMatrix) || !usrMatrix.test())
                            {   err = "the function ObjectRW() failed for the user channel  \"" + name + "\"";
                                break;  }
                               for (int j=0;j<4;j++)
                                   for (int i=0;i<4;i++)
                                       m44[i][j] = val[j * 4 + i];
                               usrMatrix.Set4(m44);
                        }
                    }
                    else
                    {
                        const char *typeName = NULL;
                        attr.TypeName((*cd).eval_index, &typeName);
                        if (typeName)   err = "the user channel  \"" + name + "\" has the unsupported data type \"" + typeName + "\"";
                        else            err = "the user channel  \"" + name + "\" has the unsupported data type \"NULL\"";
                        break;
                    }

                    // error getting value from DFG port?
                    if (retGet != 0)
                    {
                        sprintf(serr, "%ld", retGet);
                        err = "failed to get value from port \"" + name + "\" (returned " + serr + ")";
                        break;
                    }

                    // error setting value of user channel?
                    if (retSet != 0)
                    {
                        sprintf(serr, "%ld", retGet);
                        err = "failed to set value of user channel \"" + name + "\" (returned " + serr + ")";
                        break;
                    }
                }

                // error?
                if (err != "")
                {
                    feLogError(NULL, err.c_str(), err.length());
                    return;
                }
            }
            catch(FabricCore::Exception e)
            {
                feLogError(NULL, e.getDesc_cstr(), e.getDescLength());
            }
        }

        // done.
        return;
    }

    void Element::usrChannelsCollect(CLxUser_Item &item, std::vector <ChannelDef> &io_usrChannels)
    {
        //
        // this function collects all of the user channels on the
        // specified item and stores them into io_usrChannels.
        //

        // init.
        io_usrChannels.clear();
        if (!item.test())
            return;
    
        // get amount of channels.
        unsigned count = 0;
        item.ChannelCount(&count);
    
        // go through all channels and add all valid user channels to usrchn.
        for (unsigned i=0;i<count;i++)
        {
            // if the channel has a package (i.e. if it is not a user channel) then skip it.
            const char *package = NULL;
            if (LXx_OK(item.ChannelPackage(i, &package)) || package)
                continue;

            // if the channel has no type then skip it.
            const char *channel_type = NULL;
            if (!LXx_OK(item.ChannelEvalType(i, &channel_type) && channel_type))
                continue;

            // if the channel type is "none" (i.e. if it is a divider) then skip it.
            if (!strcmp(channel_type, LXsTYPE_NONE))
                continue;

            // add the channel to io_usrChannels.
            const char *name = NULL;
            item.ChannelName(i, &name);
            ChannelDef c;
            c.eval_index = -1;
            c.chan_index =  i;
            c.chan_name  = name;
            io_usrChannels.push_back(c);
        }
    }

    ChannelDef *Element::usrChannelsGetFromName(std::string channelName, std::vector <ChannelDef> &usrChannels)
    {
        for (int i=0;i<usrChannels.size();i++)
        {
            ChannelDef *c = &usrChannels[i];
            if (channelName == (*c).chan_name)
                return c;
        }
        return NULL;
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



