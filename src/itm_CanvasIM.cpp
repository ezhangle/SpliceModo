#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_FabricDFGWidget.h"
#include "_class_JSONValue.h"
#include "_class_ModoTools.h"
#include "itm_CanvasIM.h"
#include "itm_common.h"
#include <Persistence/RTValToJSONEncoder.hpp>

static CLxItemType gItemType_CanvasIM(SERVER_NAME_CanvasIM);

namespace CanvasIM
{
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
      lx::AddSpawner          (SERVER_NAME_CanvasIM ".inst", srv);
    }

    Instance()
    {
      feLog("CanvasIM::Instance::Instance() new BaseInterface");
      // init members and create base interface.
      m_baseInterface = new BaseInterface();
    };
    ~Instance()
    {
      feLog("CanvasIM::Instance::~Instance() called");
      if (m_baseInterface)
      {
        feLog("CanvasIM::Instance::~Instance() delete BaseInterface");
        // delete widget and base interface.
        FabricDFGWidget *w = FabricDFGWidget::getWidgetforBaseInterface(m_baseInterface);
        if (w) delete w;
        delete m_baseInterface;
        m_baseInterface = NULL;
      }
    };

    LxResult    pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)  LXx_OVERRIDE;
    LxResult    pins_Newborn(ILxUnknownID original, unsigned flags)         LXx_OVERRIDE  { return ItemCommon::pins_Newborn(original, flags, m_item_obj, m_baseInterface); }
    LxResult    pins_AfterLoad(void)                                        LXx_OVERRIDE  { return ItemCommon::pins_AfterLoad(m_item_obj, m_baseInterface); }
    void        pins_Doomed(void)                                           LXx_OVERRIDE  { ItemCommon::pins_Doomed(m_baseInterface); }

   public:
    ILxUnknownID   m_item_obj;
    BaseInterface *m_baseInterface;
  };

  LxResult Instance::pins_Initialize(ILxUnknownID item_obj, ILxUnknownID super)
  {
    // check Fabric env. vars.
    ModoTools::checkFabricEnvVariables(true);

    // store item ID in our member.
    m_item_obj = item_obj;

    //
    if (m_baseInterface)  m_baseInterface->m_ILxUnknownID_CanvasIM = item_obj;
    else                  feLogError("m_baseInterface == NULL");

    // done.
    return LXe_OK;
  }

  /*
    Implement the Package.
  */

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
      lx::AddServer            (SERVER_NAME_CanvasIM, srv);
    }

    Package() : m_inst_spawn (SERVER_NAME_CanvasIM ".inst") {}

    LxResult    pkg_SetupChannels   (ILxUnknownID addChan_obj)  LXx_OVERRIDE  { return ItemCommon::pkg_SetupChannels(addChan_obj, false); }
    LxResult    pkg_Attach          (void **ppvObj)             LXx_OVERRIDE  { m_inst_spawn.Alloc(ppvObj); return (ppvObj[0] ? LXe_OK : LXe_FAILED); }
    LxResult    pkg_TestInterface   (const LXtGUID *guid)       LXx_OVERRIDE  { return m_inst_spawn.TestInterfaceRC(guid); }

    LxResult    cui_UIHints         (const char *channelName, ILxUnknownID hints_obj)   LXx_OVERRIDE  { return ItemCommon::cui_UIHints(channelName, hints_obj); }

    void        sil_ItemAddChannel  (ILxUnknownID item_obj)                             LXx_OVERRIDE;
    void        sil_ItemChannelName (ILxUnknownID item_obj, unsigned int index)         LXx_OVERRIDE;

    static LXtTagInfoDesc descInfo[];

   private:
    CLxSpawner <Instance> m_inst_spawn;
  };

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

    if (item.test() && item.IsA(gItemType_CanvasIM.Type()))
    {
      if (item.GetContext(scene))
        scene.EvalModInvalidate(SERVER_NAME_CanvasIM ".mod");
    }
  }

  void Package::sil_ItemChannelName(ILxUnknownID item_obj, unsigned int index)
  {
    /*
      When a user channel's name changes, this function will be
      called. We use it to invalidate our modifier so that it's reallocated.
    */

    CLxUser_Item    item(item_obj);
    CLxUser_Scene   scene;

    if (item.test() && item.IsA(gItemType_CanvasIM.Type()))
    {
      if (item.GetContext(scene))
        scene.EvalModInvalidate(SERVER_NAME_CanvasIM ".mod");
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

  class Element : public CLxItemModifierElement
  {
   public:
    Element     (CLxUser_Evaluation &eval, ILxUnknownID item_obj);
    bool    Test(ILxUnknownID item_obj)                               LXx_OVERRIDE  { return ItemCommon::Test(item_obj, m_usrChan); }
    void    Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)  LXx_OVERRIDE;

   private:
    int                                 m_first_eval_index;
    std::vector <ModoTools::UsrChnDef>  m_usrChan;

    Instance *m_Instance;
  };

  Element::Element(CLxUser_Evaluation &eval, ILxUnknownID item_obj)
  {
    m_Instance = GetInstance(item_obj);
    BaseInterface *b = GetBaseInterface(item_obj);
    if (!b)
    { feLogError("GetBaseInterface() returned NULL");
      return; }

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
    m_first_eval_index = eval.AddChan(item, CHN_NAME_IO_FabricActive, LXfECHAN_READ);
    eval.AddChan(item, CHN_NAME_IO_FabricEval,   LXfECHAN_READ);
    char chnName[128];
    for (int i=0;i<CHN_FabricJSON_NUM;i++)
    {
      sprintf(chnName, "%s%d", CHN_NAME_IO_FabricJSON, i);
      eval.AddChan(item, chnName, LXfECHAN_READ);
    }

    // collect all the user channels and add them to eval.
    ModoTools::usrChanCollect(item, m_usrChan);
    for (unsigned i=0;i<m_usrChan.size();i++)
    {
      ModoTools::UsrChnDef &c = m_usrChan[i];

      unsigned int type;
      if      (b->HasInputPort (c.chan_name.c_str()))     type = LXfECHAN_READ;
      else if (b->HasOutputPort(c.chan_name.c_str()))     type =                 LXfECHAN_WRITE;
      else                                                type = LXfECHAN_READ | LXfECHAN_WRITE;

      c.eval_index = eval.AddChan(item, c.chan_index, type);
    }
  }

  void Element::Eval(CLxUser_Evaluation &eval, CLxUser_Attributes &attr)
  {
    // nothing to do?
    if (!eval || !attr)
      return;

    //
    BaseInterface *b = GetBaseInterface(m_Instance->m_item_obj);
    if (!b)
    { feLogError("Element::Eval(): GetBaseInterface(m_Instance->m_item_obj) returned NULL");
      return; }

    // set the base interface's evaluation member so that it doesn't
    // process notifications while the element is being evaluated.
    FTL::AutoSet<bool> isEvaluating( b->m_evaluating, true );

    // refs 'n pointers.
    FabricCore::Client *client  = b->getClient();
    if (!client)
    { feLogError("Element::Eval(): getClient() returned NULL");
      return; }
    FabricCore::DFGBinding binding = b->getBinding();
    if (!binding.isValid())
    { feLogError("Element::Eval(): invalid binding");
      return; }
    FabricCore::DFGExec graph = binding.getExec();
    if (!graph.isValid())
    { feLogError("Element::Eval(): invalid graph");
      return; }

    // read the fixed input channels and return early if the FabricActive flag is disabled.
    unsigned int eval_index = m_first_eval_index;
    int FabricActive = attr.Bool(eval_index++, false);
    int FabricEval   = attr.Int (eval_index++);
    if (!FabricActive)
      return;

    // Fabric Engine (step 1): loop through all the DFG's input ports and set
    //                         their values from the matching Modo user channels.
    {
      try
      {
        char        serr[256];
        std::string err = "";

        for (unsigned int fi=0;fi<graph.getExecPortCount();fi++)
        {
          // if the port has the wrong type then skip it.
          if (graph.getExecPortType(fi) != FabricCore::DFGPortType_In)
            continue;

          // get pointer at matching channel definition.
          const char *portName = graph.getExecPortName(fi);
          bool storable = true;

          ModoTools::UsrChnDef *cd = ModoTools::usrChanGetFromName(portName, m_usrChan);
          if (!cd)
          { err = "(step 1/3) unable to find a user channel that matches the port \"" + std::string(portName) + "\"";
            break;  }
          if (cd->eval_index < 0)
          { err = "(step 1/3) user channel evaluation index of port \"" + std::string(portName) + "\" is -1";
            break;  }

          // "DFG port value = item user channel".
          int retGet = 0;
          std::string port__resolvedType = graph.getExecPortResolvedType(fi);
          if      (   port__resolvedType == "Boolean")    {
                                                            bool val = false;
                                                            retGet = ModoTools::GetChannelValueAsBoolean(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgBoolean(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "Integer"
                   || port__resolvedType == "SInt8"
                   || port__resolvedType == "SInt16"
                   || port__resolvedType == "SInt32"
                   || port__resolvedType == "SInt64" )    {
                                                            int val = 0;
                                                            retGet = ModoTools::GetChannelValueAsInteger(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgSInt(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "Byte"
                   || port__resolvedType == "UInt8"
                   || port__resolvedType == "UInt16"
                   || port__resolvedType == "Count"
                   || port__resolvedType == "Index"
                   || port__resolvedType == "Size"
                   || port__resolvedType == "UInt32"
                   || port__resolvedType == "DataSize"
                   || port__resolvedType == "UInt64" )    {
                                                            int val = 0;
                                                            retGet = ModoTools::GetChannelValueAsInteger(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgUInt(*client, binding, portName, (uint32_t)val);
                                                          }
          else if (   port__resolvedType == "Scalar"
                   || port__resolvedType == "Float32"
                   || port__resolvedType == "Float64" )   {
                                                            double val = 0;
                                                            retGet = ModoTools::GetChannelValueAsFloat(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgFloat(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "String")     {
                                                            std::string val = "";
                                                            retGet = ModoTools::GetChannelValueAsString(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgString(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "Quat")       {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsQuaternion(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgQuat(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "Vec2")       {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsVector2(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgVec2(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "Vec3")       {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsVector3(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgVec3(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "Color")      {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsColor(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgColor(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "RGB")        {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsRGB(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgRGB(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "RGBA")       {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsRGBA(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgRGBA(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "Mat44")      {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsMatrix44(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgMat44(*client, binding, portName, val);
                                                          }
          else if (   port__resolvedType == "Xfo")        {
                                                            std::vector <double> val;
                                                            retGet = ModoTools::GetChannelValueAsXfo(attr, cd->eval_index, val);
                                                            if (retGet == 0)    BaseInterface::SetValueOfArgXfo(*client, binding, portName, val);
                                                          }
          else
          {
            storable = false;
            err = "the port \"" + std::string(portName) + "\" has the unsupported data type \"" + port__resolvedType + "\"";
            break;
          }

          if( storable ) {
            // Set ports added with a "storable type" as persistable so their values are 
            // exported if saving the graph
            // TODO: handle this in a "clean" way; here we are not in the context of an undo-able command.
            //       We would need that the DFG knows which binding types are "stored" as attributes on the
            //       DCC side and set these as persistable in the source "addPort" command.
            graph.setExecPortMetadata( portName, DFG_METADATA_UIPERSISTVALUE, "true" );
          }

          // error getting value from user channel?
          if (retGet != 0)
          {
            snprintf(serr, sizeof(serr), "%d", retGet);
            err = "failed to get value from user channel \"" + std::string(portName) + "\" (returned " + serr + ")";
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
        binding.execute();
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

        for (unsigned int fi=0;fi<graph.getExecPortCount();fi++)
        {
          // if the port has the wrong type then skip it.
          if (graph.getExecPortType(fi) != FabricCore::DFGPortType_Out)
            continue;

          // get pointer at matching channel definition.
          const char *portName = graph.getExecPortName(fi);
          ModoTools::UsrChnDef *cd = ModoTools::usrChanGetFromName(portName, m_usrChan);
          if (!cd)
          { err = "(step 3/3) unable to find a user channel that matches the port \"" + std::string(portName) + "\"";
            break;  }
          if (cd->eval_index < 0)
          { err = "(step 3/3) user channel evaluation index of port \"" + std::string(portName) + "\" is -1";
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
              retGet = BaseInterface::GetArgValueInteger(binding, portName, val);
              if (retGet == 0)
                retSet = attr.SetInt(cd->eval_index, val);
            }
            else if (dataType == LXi_TYPE_FLOAT)
            {
              double val;
              retGet = BaseInterface::GetArgValueFloat(binding, portName, val);
              if (retGet == 0)
                retSet = attr.SetFlt(cd->eval_index, val);
            }
            else if (dataType == LXi_TYPE_STRING)
            {
              std::string val;
              retGet = BaseInterface::GetArgValueString(binding, portName, val);
              if (retGet == 0)
                retSet = attr.SetString(cd->eval_index, val.c_str());
            }
            else if (dataType == LXi_TYPE_OBJECT && typeName && !strcmp (typeName, LXsTYPE_QUATERNION))
            {
              std::vector <double> val;
              retGet = BaseInterface::GetArgValueQuat(binding, portName, val);
              if (retGet == 0 && val.size() == 4)
              {
                CLxUser_Quaternion usrQuaternion;
                LXtQuaternion      q;
                if (!attr.ObjectRW(cd->eval_index, usrQuaternion) || !usrQuaternion.test())
                { err = "the function ObjectRW() failed for the user channel  \"" + std::string(portName) + "\"";
                  break;  }
                for (int i = 0; i < 4; i++)   q[i] = val[i];
                usrQuaternion.SetQuaternion(q);
              }
            }
            else if (dataType == LXi_TYPE_OBJECT && typeName && !strcmp (typeName, LXsTYPE_MATRIX4))
            {
              std::vector <double> val;
              retGet = BaseInterface::GetArgValueMat44(binding, portName, val);
              if (retGet == 0 && val.size() == 16)
              {
                CLxUser_Matrix usrMatrix;
                LXtMatrix4     m44;

                if (!attr.ObjectRW(cd->eval_index, usrMatrix) || !usrMatrix.test())
                { err = "the function ObjectRW() failed for the user channel  \"" + std::string(portName) + "\"";
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
              if (typeName)   err = "the user channel  \"" + std::string(portName) + "\" has the unsupported data type \"" + typeName + "\"";
              else            err = "the user channel  \"" + std::string(portName) + "\" has the unsupported data type \"NULL\"";
              break;
            }
          }
          else
          {
            std::vector <double> val;
            size_t N = 0;
            if (dataType == LXi_TYPE_FLOAT)
            {
              if      (cd->isVec2x)     {   N = 2;  retGet = BaseInterface::GetArgValueVec2(binding, portName, val);   }
              else if (cd->isVec3x)     {   N = 3;  retGet = BaseInterface::GetArgValueVec3(binding, portName, val);   }
              else if (cd->isRGBr)      {   N = 3;  retGet = BaseInterface::GetArgValueRGB (binding, portName, val);   }
              else if (cd->isRGBAr)     {   N = 4;  retGet = BaseInterface::GetArgValueRGBA(binding, portName, val);   }
              else
              {
                err = "something is wrong with the flags in ModoTools::UsrChnDef";
                break;
              }

              if (retGet == 0 && val.size() == N)
                for (size_t i = 0; i < N; i++)
                  if (retSet)     break;
                  else            retSet = attr.SetFlt(cd->eval_index + i, val[i]);
            }
          }

          // error getting value from DFG port?
          if (retGet != 0)
          {
            snprintf(serr, sizeof(serr), "%d", retGet);
            err = "failed to get value from DFG port \"" + std::string(portName) + "\" (returned " + serr + ")";
            break;
          }

          // error setting value of user channel?
          if (retSet != 0)
          {
            snprintf(serr, sizeof(serr), "%d", retGet);
            err = "failed to set value of user channel \"" + std::string(portName) + "\" (returned " + serr + ")";
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

  class Modifier : public CLxItemModifierServer
  {
   public:
    static void initialize()  { CLxExport_ItemModifierServer <Modifier> (SERVER_NAME_CanvasIM ".mod"); }
    const char *ItemType()  LXx_OVERRIDE  { return SERVER_NAME_CanvasIM; }
    CLxItemModifierElement *Alloc(CLxUser_Evaluation &eval, ILxUnknownID item_obj)   LXx_OVERRIDE { return new Element (eval, item_obj); }
  };

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

      if (strcmp(typeName, SERVER_NAME_CanvasIM))
        return NULL;
    }

    // get/return pointer at Instance.
    CLxLoc_PackageInstance pkg_inst(item_obj);
    if (pkg_inst.test())
    {
      CLxSpawner <Instance> spawn(SERVER_NAME_CanvasIM ".inst");
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

  // used in the plugin's initialize() function (see plugin.cpp).
  void initialize()
  {
    Instance :: initialize();
    Package  :: initialize();
    Modifier :: initialize();
  }
};  // namespace CanvasIM



