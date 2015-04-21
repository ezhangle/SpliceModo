#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_ModoTools.h"
#include "itm_dfgModoIM.h"

static CLxItemType gItemType_dfgModoIM(SERVER_NAME_dfgModoIM);

namespace dfgModoIM
{
  // The Value class implements the custom value type. The base interface for this
  // object is the Value Interface. This provides the basic functions for
  // manipulating the value. We also implement a StreamIO interface, allowing us
  // to read and write the custom value to the scene file.

  struct _JSONValue
  {
    std::string   s;
    BaseInterface *baseInterface;
    _JSONValue()
    {
      s.clear();
      baseInterface = NULL;
    }
  };

  class JSONValue : public CLxImpl_Value,
                    public CLxImpl_StreamIO
  {
   public:
    static void initialize ()
    {
      CLxGenericPolymorph *srv = NULL;

      srv = new CLxPolymorph                 <JSONValue>;
      srv->AddInterface(new CLxIfc_Value     <JSONValue>);
      srv->AddInterface(new CLxIfc_StreamIO  <JSONValue>);
      srv->AddInterface(new CLxIfc_StaticDesc<JSONValue>);

      lx::AddServer(SERVER_NAME_dfgModoIM ".jsonvalue", srv);
    }
  
    static LXtTagInfoDesc descInfo[];
    _JSONValue *m_data;

    JSONValue()   { m_data = new _JSONValue; }
    ~JSONValue()  { if (m_data) delete m_data;    }
  
    unsigned int val_Type()                               LXx_OVERRIDE { return LXi_TYPE_OBJECT; }
    LxResult     val_Copy(ILxUnknownID other)             LXx_OVERRIDE;
    LxResult     val_GetString(char *buf, unsigned len)   LXx_OVERRIDE;
    LxResult     val_SetString(const char *val)           LXx_OVERRIDE;
    void        *val_Intrinsic()                          LXx_OVERRIDE;
  
    LxResult   io_Write(ILxUnknownID stream)  LXx_OVERRIDE;
    LxResult   io_Read (ILxUnknownID stream)  LXx_OVERRIDE;
  };

  LxResult JSONValue::val_Copy(ILxUnknownID other)
  {
    /*
      Copy another instance of our custom value to this one. We just cast
      the object to our internal structure and then copy the data.
    */

    if (!m_data)    return LXe_FAILED;
    if (!other)     return LXe_FAILED;

    _JSONValue *otherData = (_JSONValue *)((void *)other);
    if (!otherData) return LXe_FAILED;
  
    m_data->s             = otherData->s;
    m_data->baseInterface = NULL;

    return LXe_OK;
  }

  LxResult JSONValue::val_GetString(char *buf, unsigned len)
  {
    /*
      This function - as the name suggests - is used to get the custom value
      as a string. We just read the string from the custom value object. As
      the caller provides a buffer and length, we should test the length of
      the buffer against our string length, and if it's too short, return
      short buffer. The caller will then provide a bigger buffer for us to
      copy the string into.
    */
  
    if (!m_data)    return LXe_FAILED;
    if (!buf)       return LXe_FAILED;

    if (m_data->s.size() >= len)
      return LXe_SHORTBUFFER;

    strncpy(buf, m_data->s.c_str(), len);

    return LXe_OK;
  }

  LxResult JSONValue::val_SetString(const char *val)
  {
    /*
      Similar to the get string function, this function sets the string.
    */

    if (!m_data)    return LXe_FAILED;
    if (!val)       return LXe_FAILED;

    m_data->s = val;
    
    return LXe_OK;
  }

  void *JSONValue::val_Intrinsic()
  {
    /*
     The Intrinsic function is the important one. This returns a pointer
     to the custom value's class, allowing callers to interface with it directly.
    */
    return (void *)m_data;
  }

  LxResult JSONValue::io_Write(ILxUnknownID stream)
  {
    /*
     The Write function is called whenever the custom value type is being
     written to a stream, for example, writing to a scene file. 

     NOTE: we do not write the string m_data->s, instead we write
           the JSON string BaseInterface::getJSON().
    */
    CLxUser_BlockWrite write(stream);
    feLog("JSONValue::io_Write()");

    if (!m_data)        return LXe_FAILED;
    if (!write.test())  return LXe_FAILED;

    // write the JSON string.
    if (!m_data->baseInterface)
    { feLogError("JSONValue::io_Write(): pointer at BaseInterface is NULL!");
      return LXe_FAILED;  }
    try
    {
      std::string json = m_data->baseInterface->getJSON();
      return write.WriteString(json.c_str());
    }
    catch (FabricCore::Exception e)
    {
      std::string err = "JSONValue::io_Write(): ";
      err += (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
      feLogError(err);
      return LXe_FAILED;
    }
  }

  LxResult JSONValue::io_Read(ILxUnknownID stream)
  {
    /*
     The Read function is called whenever the custom value type is being
     read from a stream, for example, loading from a scene file. 

     NOTE: the string is read into m_data->s and will then be used in
           the function Instance::pins_AfterLoad() to set the graph
           via BaseInterface::setFromJSON().
    */

    CLxUser_BlockRead read(stream);
    feLog("JSONValue::io_Read()");

    if (!m_data)       return LXe_FAILED;
    if (!read.test())  return LXe_FAILED;

    if (read.Read(m_data->s))
      return LXe_OK;
    else
      return LXe_FAILED;
  }

  LXtTagInfoDesc JSONValue::descInfo[] =
  {
    { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
    { 0 }
  };

  // Implement the Package and Instance. The instance doesn't have to do anything,
  // but the package adds the standard set of channels and marks certain of those
  // channels as schematic inputs/outputs. We also implement a SceneItemListener,
  // allowing us to invalidate the modifier when new channels are added.

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

    Instance()
    {
      // init members and create base interface.
      m_baseInterface = new BaseInterface();
    };

    ~Instance()
    {
      // delete widget and base interface.
      FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(m_baseInterface, false);
      if (w) delete w;
      delete m_baseInterface;
    };

    LxResult pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)   LXx_OVERRIDE;
    LxResult pins_Newborn(ILxUnknownID original, unsigned flags)          LXx_OVERRIDE;
    LxResult pins_AfterLoad(void)                                         LXx_OVERRIDE;

   public:
    ILxUnknownID   m_item_obj;        // set in pins_Initialize() and used in pins_AfterLoad(), Element::Eval(), ...
    BaseInterface *m_baseInterface;   // set in the constructor.
  };

  LxResult Instance::pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)
  {
    // store item ID in our member.
    m_item_obj = item_obj;

    // done.
    return LXe_OK;
  }

  LxResult Instance::pins_Newborn(ILxUnknownID original, unsigned flags)
  {
    /*
      This function is called when an item is added.
      We store the pointer at the BaseInterface here so that the
      functions JSONValue::io_Write() can write the JSON string
      when the scene is saved.

      note: this function is *not* called when a scene is loaded,
            instead pins_AfterLoad() is called.
    */

    // store pointer at BaseInterface in JSON channel.
    bool ok = false;
    CLxUser_Item item(m_item_obj);
    if (item.test())
    {
      CLxUser_ChannelWrite chanWrite;
      if (chanWrite.from(item))
      {
        CLxUser_Value value_json;
        if (chanWrite.Object(item, CHN_NAME_IO_FabricJSON, value_json) && value_json.test())
        {
          _JSONValue *jv = (_JSONValue *)value_json.Intrinsic();
          if (jv)
          {
            ok = true;
            jv->baseInterface = m_baseInterface;
          }
        }
      }
    }
    if (!ok)
      feLogError("failed to store pointer at BaseInterface in JSON channel");

    // done.
    return LXe_OK;
  }

  LxResult Instance::pins_AfterLoad(void)
  {
    /*
      This function is called when a scene was loaded.

      We store the pointer at the BaseInterface here so that the
      functions JSONValue::io_Write() can write the JSON string
      when the scene is saved.

      Furthermore we set the graph from the content (i.e. the string)
      of the channel CHN_NAME_IO_FabricJSON.
    */

    // init err string,
    std::string err = "pins_AfterLoad() failed: ";

    // get BaseInterface.
    BaseInterface *b = m_baseInterface;

    // create item.
    CLxUser_Item item(m_item_obj);
    if (!item.test())
    { err += "item(m_item) failed";
      feLogError(err);
      return LXe_OK;  }

    // log.
    std::string itemName;
    item.GetUniqueName(itemName);
    std::string info;
    info = "item \"" + itemName + "\": setting Fabric base interface from JSON string.";
    feLog(0, info.c_str(), info.length());

    // create channel reader.
    CLxUser_ChannelRead chanRead;
    if (!chanRead.from(item))
    { err += "failed to create channel reader.";
      feLogError(err);
      return LXe_OK;  }

    // get value object.
    CLxUser_Value value;
    if (!chanRead.Object(item, CHN_NAME_IO_FabricJSON, value) || !value.test())
    { // note: we don't log an error here.
      return LXe_OK;  }

    // get content of channel CHN_NAME_IO_FabricJSON.
    _JSONValue *jv = (_JSONValue *)value.Intrinsic();
    if (!jv)
    { err += "channel \"" CHN_NAME_IO_FabricJSON "\" data is NULL";
      feLogError(err);
      return LXe_OK;  }

    // set pointer at BaseInterface.
    jv->baseInterface = m_baseInterface;

    // do it.
    try
    {
      if (jv->s.length() > 0)
        b->setFromJSON(jv->s);
    }
    catch (FabricCore::Exception e)
    {
      err += (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
      feLogError(err);
    }

    // done.
    return LXe_OK;
  }

  Instance *GetInstance(ILxUnknownID item_obj)
  {
    CLxUser_Item item(item_obj);
    if (!item.test())
      return NULL;

    // check the type.
    {
      const char *typeName = NULL;
      CLxUser_SceneService srv;
      if (srv.ItemTypeName(item.Type(), &typeName) != LXe_OK || !typeName)
        return NULL;

      const unsigned int numBytes = __min(strlen(typeName), strlen(SERVER_NAME_dfgModoIM));
      if (memcmp(typeName, SERVER_NAME_dfgModoIM, numBytes))
        return NULL;
    }

    // get/return pointer at Instance.
    CLxLoc_PackageInstance pkg_inst(item_obj);
    if (pkg_inst.test())
    {
      CLxSpawner <Instance> spawn(SERVER_NAME_dfgModoIM ".inst");
      return spawn.Cast(pkg_inst);
    }
    return NULL;
  }

  BaseInterface *GetBaseInterface(ILxUnknownID item_obj)
  {
    Instance *inst = GetInstance(item_obj);
    if (inst)   return inst->m_baseInterface;
    else        return NULL;
  }

  void InvalidateItem(void *ILxUnknownID_item_obj)
  {
    // invalidates an item so that it gets re-evaluated.

    if (ILxUnknownID_item_obj)
    {
      ILxUnknownID item_obj = (ILxUnknownID)ILxUnknownID_item_obj;
      CLxUser_Item item(item_obj);

      if (item.test() && item.IsA(gItemType_dfgModoIM.Type()))
      {
        static int evalCounter = 0;

        char cmd[1024];
        std::string err;
        std::string uniqueName;
        snprintf(cmd, sizeof(cmd), "item.channel %s %ld item:\"%s\"", CHN_NAME_IO_FabricEval, evalCounter++, item.IdentPtr());
        ModoTools::ExecuteCommand(std::string(cmd), err);
      }
    }
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
      Add some basic built in channels.
    */

    CLxUser_AddChannel  add_chan(addChan_obj);
    LxResult            result = LXe_FAILED;

    if (add_chan.test())
    {
      add_chan.NewChannel(CHN_NAME_IO_FabricActive, LXsTYPE_BOOLEAN);
      add_chan.SetDefault(1, 1);

      add_chan.NewChannel(CHN_NAME_IO_FabricEval, LXsTYPE_INTEGER);
      add_chan.SetDefault(0, 0);
      add_chan.SetInternal();

      add_chan.NewChannel(CHN_NAME_IO_FabricJSON, "+" SERVER_NAME_dfgModoIM ".jsonvalue");
      add_chan.SetStorage("+" SERVER_NAME_dfgModoIM ".jsonvalue");
      add_chan.SetInternal();

      result = LXe_OK;
    }

    return result;
  }

  LxResult Package::pkg_Attach(void **ppvObj)
  {
    /*
      Allocate an instance of the package instance.
    */

    m_inst_spawn.Alloc(ppvObj);

    return (ppvObj[0] ? LXe_OK : LXe_FAILED);
  }

  LxResult Package::pkg_TestInterface(const LXtGUID *guid)
  {
    /*
      This is called for the various interfaces this package could
      potentially support, it should return a result code to indicate
      if it implements the specified guid.
     */

    return m_inst_spawn.TestInterfaceRC(guid);
  }

  LxResult Package::cui_UIHints(const char *channelName, ILxUnknownID hints_obj)
  {
    // WIP: we must be able to somehow access Instance, so that we can get
    // the BaseInterface and the DFG input/output ports and set the channel
    // hints accordingly.

    /*
      Here we set some hints for the built in channels. These allow channels
      to be displayed as either inputs or outputs in the schematic. 
    */

    CLxUser_UIHints hints(hints_obj);
    LxResult        result = LXe_FAILED;

    if (hints.test())
    {
      if (strcmp(channelName, "draw"))
      {
          if (   !strcmp(channelName, CHN_NAME_IO_FabricActive)
              || !strcmp(channelName, CHN_NAME_IO_FabricEval)
              || !strcmp(channelName, CHN_NAME_IO_FabricJSON)
             )
          {
            result = hints.ChannelFlags(0);   // by default we don't display the fixed channels in the schematic view.
          }
          else
          {
            // WIP: must be able to somehow access Instance.


            //if      ((*quickhack_baseInterface).HasInputPort(channelName))  result = hints.ChannelFlags(LXfUIHINTCHAN_INPUT_ONLY  | LXfUIHINTCHAN_SUGGESTED);
            //else if ((*quickhack_baseInterface).HasOutputPort(channelName)) result = hints.ChannelFlags(LXfUIHINTCHAN_OUTPUT_ONLY | LXfUIHINTCHAN_SUGGESTED);
            //else                                                            result = hints.ChannelFlags(0);
          }
      }
      result = LXe_OK;
    }

    return result;
  }

  void Package::sil_ItemAddChannel(ILxUnknownID item_obj)
  {
    /*
      When user channels are added to our item type, this function will be
      called. We use it to invalidate our modifier so that it's reallocated.
      We don't need to worry about channels being removed, as the evaluation
      system will automatically invalidate the modifier when channels it
      writes are removed.
    */

    CLxUser_Item    item(item_obj);
    CLxUser_Scene   scene;

    if (item.test() && item.IsA(gItemType_dfgModoIM.Type()))
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
    Implement the Modifier Element and Server. This reads the input channels as
    read only channels and output channels as write only channels.
  */

  struct ChannelDef
  {
    int  chan_index;            // item channel index.
    int  eval_index;            // evaluation index.
    std::string chan_name;      // name of the channnel.
    bool isSingleton;           // true: all other "is*" flags are equal false.
    bool isVec2x;               // true: this is the first channel of a 2D vector.
    bool isVec3x;               // true: this is the first channel of a 3D vector.
    bool isRGBr;                // true: this is the first channel of a RGB color.
    bool isRGBAr;               // true: this is the first channel of a RGBA color.

    ChannelDef () : chan_index(-1), eval_index(-1), chan_name(""),
                    isSingleton(true),
                    isVec2x(false), isVec3x(false), isRGBr(false), isRGBAr(false) {}
  };

  class Element : public CLxItemModifierElement
  {
   public:
    Element(CLxUser_Evaluation &eval, ILxUnknownID item_obj);
    bool    Test(ILxUnknownID item_obj)                                LXx_OVERRIDE;
    void    Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)   LXx_OVERRIDE;

   private:
    int                      m_eval_index_FabricActive;
    int                      m_eval_index_FabricJSON;
    std::vector <ChannelDef> m_usrChan;
    void        usrChanCollect    (CLxUser_Item &item, std::vector <ChannelDef> &io_usrChan);
    ChannelDef *usrChanGetFromName(std::string channelName, std::vector <ChannelDef> &usrChan);

    Instance *m_Instance;
  };

  class Modifier : public CLxItemModifierServer
  {
   public:
    static void initialize()
    {
        CLxExport_ItemModifierServer <Modifier> (SERVER_NAME_dfgModoIM ".mod");
    }

    const char  *ItemType()   LXx_OVERRIDE;

    CLxItemModifierElement *Alloc(CLxUser_Evaluation &eval, ILxUnknownID item_obj)   LXx_OVERRIDE;
  };

  Element::Element(CLxUser_Evaluation &eval, ILxUnknownID item_obj)
  {
    m_Instance = GetInstance(item_obj);
    BaseInterface *b = GetBaseInterface(item_obj);
    if (!b)
    { feLogError("GetBaseInterface() returned NULL");
      return; }
    b->m_ILxUnknownID_dfgModoIM = item_obj;

    /*
      In the constructor, we want to add the input and output channels
      required for this modifier. The inputs are hardcoded, but for the
      outputs, we want to scan through all the user channels on the current
      item and add those. We cache the user channels, so we can easily check
      when they've changed.
    */

    CLxUser_Item item(item_obj);
    if (!item.test())
      return;

    // add the fixed input channels to eval.
    m_eval_index_FabricActive = eval.AddChan(item, CHN_NAME_IO_FabricActive, LXfECHAN_READ);
    m_eval_index_FabricActive = eval.AddChan(item, CHN_NAME_IO_FabricEval,   LXfECHAN_READ);
    m_eval_index_FabricJSON   = eval.AddChan(item, CHN_NAME_IO_FabricJSON,   LXfECHAN_READ);

    // collect all the user channels and add them to eval.
    usrChanCollect(item, m_usrChan);
    for (unsigned i = 0; i < m_usrChan.size(); i++)
    {
      ChannelDef &c = m_usrChan[i];

      unsigned int type;
      if      (b->HasInputPort (c.chan_name.c_str()))     type = LXfECHAN_READ;
      else if (b->HasOutputPort(c.chan_name.c_str()))     type =                 LXfECHAN_WRITE;
      else                                                type = LXfECHAN_READ | LXfECHAN_WRITE;

      c.eval_index = eval.AddChan(item, c.chan_index, type);
    }
  }

  bool Element::Test(ILxUnknownID item_obj)
  {
    /*
      When the list of user channels for a particular item changes, the
      modifier will be invalidated. This function will be called to check
      if the modifier we allocated previously matches what we'd allocate
      if the Alloc function was called now. We return true if it does.
    */

    CLxUser_Item             item(item_obj);
    std::vector <ChannelDef> tmp;

    if (item.test())
    {
      usrChanCollect(item, tmp);

      if (tmp.size() == m_usrChan.size())
      {
        bool foundDifference = false;
        for (int i = 0; i < tmp.size(); i++)
          if (memcmp(&tmp[i], &m_usrChan[i], sizeof(ChannelDef)))
          {
            foundDifference = true;
            break;
          }
        return !foundDifference;
      }
    }

    return false;
  }

  void Element::Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)
  {
    // nothing to do?
    if (!eval || !attr)
      return;

    //
    BaseInterface *b = GetBaseInterface(m_Instance->m_item_obj);
    if (!b)
    { feLogError("Element::Eval(): GetBaseInterface(m_Instance->m_item) returned NULL");
      return; }

    // refs at DFG wrapper members.
    FabricCore::Client                            *client  = b->getClient();
    if (!client)
    { feLogError("Element::Eval(): getClient() returned NULL");
      return; }
    FabricServices::DFGWrapper::Binding           *binding = b->getBinding();
    if (!binding)
    { feLogError("Element::Eval(): getBinding() returned NULL");
      return; }
    FabricServices::DFGWrapper::GraphExecutablePtr graph   = DFGWrapper::GraphExecutablePtr::StaticCast(binding->getExecutable());
    if (graph.isNull())
    { feLogError("Element::Eval(): getExecutable() returned NULL");
      return; }

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
        FabricServices::DFGWrapper::PortList portlist = graph->getPorts();

        for (int fi = 0; fi < portlist.size(); fi++)
        {
          // get port.
          FabricServices::DFGWrapper::PortPtr port = portlist[fi];
          if (port.isNull())  continue;

          // if the port has the wrong type then skip it.
          if (port->getPortType() != FabricCore::DFGPortType_In)
            continue;

          // get pointer at matching channel definition.
          ChannelDef *cd = usrChanGetFromName(port->getName(), m_usrChan);
          if (!cd || cd->eval_index < 0)
          { err  = "unable to find a user channel that matches the port \"" + std::string(port->getName()) + "\"";
            break;  }

          // "DFG port value = item user channel".
          int retGet = 0;
          std::string port__resolvedType = port->getResolvedType();
          if      (   port__resolvedType == "Boolean")    {
                                                            bool val = false;
                                                            retGet = ModoTools::GetChannelValueAsBoolean(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortBoolean(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "SInt8"
                   || port__resolvedType == "SInt16"
                   || port__resolvedType == "SInt32"
                   || port__resolvedType == "SInt64" )    {
                                                            int val = 0;
                                                            retGet = ModoTools::GetChannelValueAsInteger(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortSInt(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "UInt8"
                   || port__resolvedType == "UInt16"
                   || port__resolvedType == "UInt32"
                   || port__resolvedType == "UInt64" )    {
                                                            unsigned int val = 0;
                                                            retGet = ModoTools::GetChannelValueAsInteger(attr, cd->eval_index, *(int *)val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortUInt(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "Float32"
                   || port__resolvedType == "Float64" )   {
                                                            double val = 0;
                                                            retGet = ModoTools::GetChannelValueAsFloat(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortFloat(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "String")     {
                                                            std::string val = "";
                                                            retGet = ModoTools::GetChannelValueAsString(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortString(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "Quat")       {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsQuaternion(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortQuat(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "Vec2")       {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsVector2(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortVec2(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "Vec3")       {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsVector3(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortVec3(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "Color")      {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsColor(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortColor(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "RGB")        {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsRGB(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortRGB(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "RGBA")       {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsRGBA(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortRGBA(*client, *binding, port, val);
                                                          }
          else if (   port__resolvedType == "Mat44")      {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsMatrix44(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfPortMat44(*client, *binding, port, val);
                                                          }
          else
          {
            err = "the port \"" + std::string(port->getName()) + "\" has the unsupported data type \"" + port__resolvedType + "\"";
            break;
          }

          // error getting value from user channel?
          if (retGet != 0)
          {
            snprintf(serr, sizeof(serr), "%ld", retGet);
            err = "failed to get value from user channel \"" + std::string(port->getName()) + "\" (returned " + serr + ")";
            break;
          }
        }

        // error?
        if (err != "")
        {
          feLogError(err);
          return;
        }
      }
      catch (FabricCore::Exception e)
      {
        std::string s = std::string("Element::Eval()(step 1): ") + (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
        feLogError(s);
      }
    }

    // Fabric Engine (step 2): execute the DFG.
    {
      try
      {
        binding->execute();
      }
      catch (FabricCore::Exception e)
      {
        std::string s = std::string("Element::Eval()(step 2): ") + (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
        feLogError(s);
      }
    }

    // Fabric Engine (step 3): loop through all the DFG's output ports and set
    //                         the values of the matching Modo user channels.
    {
      try
      {
        char        serr[256];
        std::string err = "";
        FabricServices::DFGWrapper::PortList portlist = graph->getPorts();

        for (int fi = 0; fi < portlist.size(); fi++)
        {
          // get port.
          if (portlist[fi].isNull())  continue;
          FabricServices::DFGWrapper::PortPtr port = portlist[fi];

          // if the port has the wrong type then skip it.
          if (port->getPortType() != FabricCore::DFGPortType_Out)
            continue;

          // get pointer at matching channel definition.
          std::string name = port->getName();
          ChannelDef *cd = usrChanGetFromName(name, m_usrChan);
          if (!cd || cd->eval_index < 0)
          { err = "unable to find a user channel that matches the port \"" + name + "\"";
            break;  }

          // "item user channel = DFG port value".
          int dataType = attr.Type(cd->eval_index);
          const char *typeName = NULL;
          if (!LXx_OK(attr.TypeName(cd->eval_index, &typeName)))
            typeName = NULL;
          FabricCore::RTVal rtval;
          int retGet = 0;
          int retSet = LXe_OK;
          if (cd->isSingleton)
          {
            if      (dataType == LXi_TYPE_INTEGER)
            {
              int val;
              retGet = BaseInterface::GetPortValueInteger(port, val);
              if (retGet == 0)
                retSet = attr.SetInt(cd->eval_index, val);
            }
            else if (dataType == LXi_TYPE_FLOAT)
            {
              double val;
              retGet = BaseInterface::GetPortValueFloat(port, val);
              if (retGet == 0)
                retSet = attr.SetFlt(cd->eval_index, val);
            }
            else if (dataType == LXi_TYPE_STRING)
            {
              std::string val;
              retGet = BaseInterface::GetPortValueString(port, val);
              if (retGet == 0)
                retSet = attr.SetString(cd->eval_index, val.c_str());
            }
            else if (dataType == LXi_TYPE_OBJECT && typeName && !strcmp (typeName, LXsTYPE_QUATERNION))
            {
              std::vector <double> val;
              retGet = BaseInterface::GetPortValueQuat(port, val);
              if (retGet == 0 && val.size() == 4)
              {
                CLxUser_Quaternion usrQuaternion;
                LXtQuaternion      q;
                if (!attr.ObjectRW(cd->eval_index, usrQuaternion) || !usrQuaternion.test())
                { err = "the function ObjectRW() failed for the user channel  \"" + name + "\"";
                  break;  }
                for (int i = 0; i < 4; i++)   q[i] = val[i];
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

                if (!attr.ObjectRW(cd->eval_index, usrMatrix) || !usrMatrix.test())
                { err = "the function ObjectRW() failed for the user channel  \"" + name + "\"";
                  break;  }

                for (int j = 0; j < 4; j++)
                  for (int i = 0; i < 4; i++)
                    m44[i][j] = val[j * 4 + i];

                usrMatrix.Set4(m44);
              }
            }
            else
            {
              const char *typeName = NULL;
              attr.TypeName(cd->eval_index, &typeName);
              if (typeName)   err = "the user channel  \"" + name + "\" has the unsupported data type \"" + typeName + "\"";
              else            err = "the user channel  \"" + name + "\" has the unsupported data type \"NULL\"";
              break;
            }
          }
          else
          {
            std::vector <double> val;
            int N = 0;
            if (dataType == LXi_TYPE_FLOAT)
            {
              if      (cd->isVec2x)     {   N = 2;  retGet = BaseInterface::GetPortValueVec2(port, val);   }
              else if (cd->isVec3x)     {   N = 3;  retGet = BaseInterface::GetPortValueVec3(port, val);   }
              else if (cd->isRGBr)      {   N = 3;  retGet = BaseInterface::GetPortValueRGB (port, val);   }
              else if (cd->isRGBAr)     {   N = 4;  retGet = BaseInterface::GetPortValueRGBA(port, val);   }
              else
              {
                err = "something is wrong with the flags in ChannelDef";
                break;
              }

              if (retGet == 0 && val.size() == N)
                for (int i = 0; i < N; i++)
                  if (retSet)     break;
                  else            retSet = attr.SetFlt(cd->eval_index + i, val[i]);
            }
          }

          // error getting value from DFG port?
          if (retGet != 0)
          {
            snprintf(serr, sizeof(serr), "%ld", retGet);
            err = "failed to get value from DFG port \"" + name + "\" (returned " + serr + ")";
            break;
          }

          // error setting value of user channel?
          if (retSet != 0)
          {
            snprintf(serr, sizeof(serr), "%ld", retGet);
            err = "failed to set value of user channel \"" + name + "\" (returned " + serr + ")";
            break;
          }
        }

        // error?
        if (err != "")
        {
          feLogError(err);
          return;
        }
      }
      catch (FabricCore::Exception e)
      {
        std::string s = std::string("Element::Eval()(step 3): ") + (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
        feLogError(s);
      }
    }

    // done.
    return;
  }

  void Element::usrChanCollect(CLxUser_Item &item, std::vector <ChannelDef> &io_usrChan)
  {
    /*
      this function collects all of the user channels on the
      specified item and stores them into io_usrChan.
    */

    // init.
    io_usrChan.clear();
    if (!item.test())
      return;

    // get amount of channels.
    unsigned count = 0;
    item.ChannelCount(&count);

    // go through all channels and add all valid user channels to io_usrChan.
    for (unsigned i = 0; i < count; i++)
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

      // add the channel to io_usrChan.
      const char *name = NULL;
      item.ChannelName(i, &name);
      ChannelDef c;
      c.eval_index = -1;
      c.chan_index =  i;
      c.chan_name  = name;
      io_usrChan.push_back(c);
    }

    // go through io_usrChan and set the isVec2, isVec3, etc. flags.
    for (int i = 0; i < io_usrChan.size(); i++)
    {
      ChannelDef          &c    = io_usrChan[i];
      const std::string   &name = c.chan_name;

      //
      int idx = i;

      // check if we have a 2D or 3D vector.
      if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".X") != std::string::npos)
      {
        idx++;
        if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".Y") != std::string::npos)
        {
          idx++;
          if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".Z") != std::string::npos) c.isVec3x = true;
          else                                                                                       c.isVec2x = true;
        }
      }

      // check if we have a color.
      if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".R") != std::string::npos)
      {
        idx++;
        if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".G") != std::string::npos)
        {
          idx++;
          if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".B") != std::string::npos)
          {
            idx++;
            if (idx < io_usrChan.size() && io_usrChan[idx].chan_name.rfind(".A") != std::string::npos) c.isRGBAr = true;
            else                                                                                       c.isRGBr  = true;
          }
        }
      }

      // set singleton flag.
      c.isSingleton = (   !c.isVec2x
                        && !c.isVec3x
                        && !c.isRGBr
                        && !c.isRGBAr);
    }
  }

  ChannelDef *Element::usrChanGetFromName(std::string channelName, std::vector <ChannelDef> &usrChan)
  {
    // "normal" channel?
    for (int i = 0; i < usrChan.size(); i++)
    {
      ChannelDef *c = &usrChan[i];
      if (channelName == c->chan_name)
        return c;
    }

    // vector/color/etc. channel?
    for (int i = 0; i < usrChan.size(); i++)
    {
      ChannelDef *c = &usrChan[i];
      if (channelName + ".X" == c->chan_name)   return c;
      if (channelName + ".R" == c->chan_name)   return c;
      if (channelName + ".U" == c->chan_name)   return c;
    }

    // not found.
    return NULL;
  }

  const char *Modifier::ItemType()
  {
    /*
      The modifier should only associate itself with this item type.
    */

    return SERVER_NAME_dfgModoIM;
  }

  CLxItemModifierElement *Modifier::Alloc (CLxUser_Evaluation &eval, ILxUnknownID item)
  {
    /*
      Allocate and return the modifier element.
    */
    return new Element (eval, item);
  }

  // used in the plugin's initialize() function (see plugin.cpp).
  void initialize()
  {
    JSONValue::initialize();
    Instance ::initialize();
    Package  ::initialize();
    Modifier ::initialize();
  }
};  // namespace dfgModoIM



