#include "_class_BaseInterface.h"
#include "_class_ModoTools.h"

using namespace FabricServices;

FabricCore::Client BaseInterface::s_client;
DFGWrapper::Host * BaseInterface::s_host = NULL;
FabricServices::ASTWrapper::KLASTManager * BaseInterface::s_manager = NULL;
FabricServices::Commands::CommandStack BaseInterface::s_stack;
unsigned int BaseInterface::s_maxId = 0;
void (*BaseInterface::s_logFunc)(void *, const char *, unsigned int) = NULL;
void (*BaseInterface::s_logErrorFunc)(void *, const char *, unsigned int) = NULL;
std::map<unsigned int, BaseInterface*> BaseInterface::s_instances;

BaseInterface::BaseInterface()
{
  m_id                  = s_maxId++;
  m_item_obj_dfgModoIM  = NULL;

  // construct the client
  if(s_instances.size() == 0)
  {
    try
    {
      // create a client
      FabricCore::Client::CreateOptions options;
      memset( &options, 0, sizeof( options ) );
      options.optimizationType = FabricCore::ClientOptimizationType_Background;
      s_client = FabricCore::Client(&logFunc, NULL, &options);
      s_client.loadExtension("Math",     "", false);
      s_client.loadExtension("Geometry", "", false);

      // create a host for Canvas
      s_host = new DFGWrapper::Host(s_client);

      // create an empty binding
      m_binding = s_host->createBindingToNewGraph();

    // create KL AST manager
      s_manager = new ASTWrapper::KLASTManager(&s_client);

    // command stack
    s_stack;

    // set the graph on the view
      setGraph(m_binding.getGraph());
    }
    catch(FabricCore::Exception e)
    {
      logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
  }

  s_instances.insert(std::pair<unsigned int, BaseInterface*>(m_id, this));
}

BaseInterface::~BaseInterface()
{
  std::map<unsigned int, BaseInterface*>::iterator it = s_instances.find(m_id);

  m_binding = DFGWrapper::Binding();

  if(it != s_instances.end())
  {
    s_instances.erase(it);
    if(s_instances.size() == 0)
    {
      try
      {
        printf("Destructing client...\n");
    s_stack.clear();
    delete(s_manager);
        delete(s_host);
        s_client = FabricCore::Client();
      }
      catch(FabricCore::Exception e)
      {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
      }
    }
  }
}

unsigned int BaseInterface::getId()
{
  return m_id;
}

BaseInterface * BaseInterface::getFromId(unsigned int id)
{
  std::map<unsigned int, BaseInterface*>::iterator it = s_instances.find(id);
  if(it == s_instances.end())
    return NULL;
  return it->second;
}

FabricCore::Client * BaseInterface::getClient()
{
  return &s_client;
}

FabricServices::DFGWrapper::Host * BaseInterface::getHost()
{
  return s_host;
}

FabricServices::DFGWrapper::Binding * BaseInterface::getBinding()
{
  return &m_binding;
}

FabricServices::ASTWrapper::KLASTManager * BaseInterface::getManager()
{
  return s_manager;
}

FabricServices::Commands::CommandStack * BaseInterface::getStack()
{
  return &s_stack;
}

std::string BaseInterface::getJSON()
{
  try
  {
    return m_binding.getGraph().exportJSON();
  }
  catch(FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
  return "";
}

void BaseInterface::setFromJSON(const std::string & json)
{
  try
  {
    m_binding = s_host->createBindingFromJSON(json.c_str());
    setGraph(m_binding.getGraph());
  }
  catch(FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::setLogFunc(void (*in_logFunc)(void *, const char *, unsigned int))
{
  s_logFunc = in_logFunc;
}

void BaseInterface::setLogErrorFunc(void (*in_logErrorFunc)(void *, const char *, unsigned int))
{
  s_logErrorFunc = in_logErrorFunc;
}

void BaseInterface::onPortInserted(FabricServices::DFGWrapper::Port port)
{
    try
    {
        std::string err;
        CLxUser_Item item((ILxUnknownID)m_item_obj_dfgModoIM);
        if (!item.test())
        {   err = "item((ILxUnknownID)m_item_obj_dfgModoIM) failed";
            logErrorFunc(0, err.c_str(), err.length());
            return;    }

        std::string dataType   = port.getDataType();
        std::string structType = "";

        if      (   dataType == "")         {   err = "port.getDataType() == \"\"";
                                                logErrorFunc(0, err.c_str(), err.length());
                                                return;    }

        else if (   dataType == "Boolean")  {   dataType = "boolean";                           }

        else if (   dataType == "Integer"
                 || dataType == "SInt8"
                 || dataType == "SInt16"
                 || dataType == "SInt32"
                 || dataType == "SInt64"
                 || dataType == "UInt8"
                 || dataType == "UInt16"
                 || dataType == "UInt32"
                 || dataType == "UInt64")   {   dataType = "integer";                           }

        else if (   dataType == "Scalar"
                 || dataType == "Float32"
                 || dataType == "Float64")  {   dataType = "float";                             }

        else if (   dataType == "String")   {   dataType = "string";                            }

        else if (   dataType == "Quat")     {   dataType = "quaternion";                        }

        else if (   dataType == "Mat44")    {   dataType = "matrix";                            }

        else if (   dataType == "Vec2")     {   dataType = "float";     structType = "vecXY";   }
        else if (   dataType == "Vec3")     {   dataType = "float";     structType = "vecXYZ";  }

        else if (   dataType == "RGB")      {   dataType = "float";     structType = "vecRGB";  }
        else if (   dataType == "RGBA")     {   dataType = "float";     structType = "vecRGBA"; }

        else
        {
            err = "unable to create user channel, type \"" + dataType + "\" not yet implemented";
            logErrorFunc(0, err.c_str(), err.length());
            return;
        }

        if (!ModoTools::CreateUserChannel(&item, port.getName(), dataType, structType, err))
            logErrorFunc(0, err.c_str(), err.length());
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::onPortRemoved(FabricServices::DFGWrapper::Port port)
{
    try
    {
        std::string err;
        CLxUser_Item item((ILxUnknownID)m_item_obj_dfgModoIM);
        if (!item.test())
        {   err = "item((ILxUnknownID)m_item_obj_dfgModoIM) failed";
            logErrorFunc(0, err.c_str(), err.length());
            return;    }

        if (!ModoTools::DeleteUserChannel(&item, port.getName(), err))
            logErrorFunc(0, err.c_str(), err.length());
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::onPortRenamed(FabricServices::DFGWrapper::Port port, const char *oldName)
{
    try
    {
        std::string err;
        CLxUser_Item item((ILxUnknownID)m_item_obj_dfgModoIM);
        if (!item.test())
        {   err = "item((ILxUnknownID)m_item_obj_dfgModoIM) failed";
            logErrorFunc(0, err.c_str(), err.length());
            return;    }

        if (!ModoTools::RenameUserChannel(&item, std::string(oldName), port.getName(), err))
            logErrorFunc(0, err.c_str(), err.length());
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::logFunc(void * userData, const char * message, unsigned int length)
{
  if (s_logFunc)
  {
    s_logFunc(userData, message, length);
  }
  else
  {
    printf("BaseInterface: %s\n", message);
  }
}

void BaseInterface::logErrorFunc(void * userData, const char * message, unsigned int length)
{
  if (s_logErrorFunc)
  {
    s_logErrorFunc(userData, message, length);
  }
  else
  {
    printf("BaseInterface: error: %s\n", message);
  }
}

bool BaseInterface::HasInputPort(const char *portName)
{
    try
    {
        FabricServices::DFGWrapper::Port port = getGraph().getPort(portName);
        return (port.isValid() && port.getPortType() == FabricCore::DFGPortType_In);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return false;
    }
}

bool BaseInterface::HasInputPort(const std::string &portName)
{
    return HasInputPort(portName.c_str());
}

bool BaseInterface::HasOutputPort(const char *portName)
{
    try
    {
        FabricServices::DFGWrapper::Port port = getGraph().getPort(portName);
        return (port.isValid() && port.getPortType() == FabricCore::DFGPortType_Out);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return false;
    }
}

bool BaseInterface::HasOutputPort(const std::string &portName)
{
    return HasOutputPort(portName.c_str());
}

int BaseInterface::GetPortValueBoolean(FabricServices::DFGWrapper::Port &port, bool &out, bool strict)
{
    // init output.
    out = false;

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)    return -1;

        else if (dataType == "Boolean")     out = rtval.getBoolean();

        else if (!strict)
        {
            if      (dataType == "Float32") out = (0 != rtval.getFloat32());
            else if (dataType == "Float64") out = (0 != rtval.getFloat64());

            else if (dataType == "SInt8")   out = (0 != rtval.getSInt8());
            else if (dataType == "SInt16")  out = (0 != rtval.getSInt16());
            else if (dataType == "SInt32")  out = (0 != rtval.getSInt32());
            else if (dataType == "SInt64")  out = (0 != rtval.getSInt64());

            else if (dataType == "UInt8")   out = (0 != rtval.getUInt8());
            else if (dataType == "UInt16")  out = (0 != rtval.getUInt16());
            else if (dataType == "UInt32")  out = (0 != rtval.getUInt32());
            else if (dataType == "UInt64")  out = (0 != rtval.getUInt64());

            else return -1;
        }
        else return -1;
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueInteger(FabricServices::DFGWrapper::Port &port, int &out, bool strict)
{
    // init output.
    out = 0;

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)    return -1;

        else if (dataType == "SInt8")       out = (int)rtval.getSInt8();
        else if (dataType == "SInt16")      out = (int)rtval.getSInt16();
        else if (dataType == "SInt32")      out = (int)rtval.getSInt32();
        else if (dataType == "SInt64")      out = (int)rtval.getSInt64();

        else if (dataType == "UInt8")       out = (int)rtval.getUInt8();
        else if (dataType == "UInt16")      out = (int)rtval.getUInt16();
        else if (dataType == "UInt32")      out = (int)rtval.getUInt32();
        else if (dataType == "UInt64")      out = (int)rtval.getUInt64();

        else if (!strict)
        {
            if      (dataType == "Boolean") out = (int)rtval.getBoolean();

            else if (dataType == "Float32") out = (int)rtval.getFloat32();
            else if (dataType == "Float64") out = (int)rtval.getFloat64();

            else return -1;
        }
        else return -1;
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueFloat(FabricServices::DFGWrapper::Port &port, double &out, bool strict)
{
    // init output.
    out = 0;

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)    return -1;

        else if (dataType == "Float32")     out = (double)rtval.getFloat32();
        else if (dataType == "Float64")     out = (double)rtval.getFloat64();

        else if (!strict)
        {
            if      (dataType == "Boolean") out = (double)rtval.getBoolean();

            else if (dataType == "SInt8")   out = (double)rtval.getSInt8();
            else if (dataType == "SInt16")  out = (double)rtval.getSInt16();
            else if (dataType == "SInt32")  out = (double)rtval.getSInt32();
            else if (dataType == "SInt64")  out = (double)rtval.getSInt64();

            else if (dataType == "UInt8")   out = (double)rtval.getUInt8();
            else if (dataType == "UInt16")  out = (double)rtval.getUInt16();
            else if (dataType == "UInt32")  out = (double)rtval.getUInt32();
            else if (dataType == "UInt64")  out = (double)rtval.getUInt64();

            else return -1;
        }
        else return -1;
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueString(FabricServices::DFGWrapper::Port &port, std::string &out, bool strict)
{
    // init output.
    out = "";

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)    return -1;

        else if (dataType == "String")      out = rtval.getStringCString();

        else if (!strict)
        {
            char    s[64];
            bool    b;
            int     i;
            double  f;

            if (GetPortValueBoolean(port, b, true) == 0)
            {
                out = (b ? "true" : "false");
                return 0;
            }

            if (GetPortValueInteger(port, i, true) == 0)
            {
                sprintf(s, "%ld", i);
                out = s;
                return 0;
            }

            if (GetPortValueFloat(port, f, true) == 0)
            {
                sprintf(s, "%f", f);
                out = s;
                return 0;
            }

            return -1;
        }
        else
            return -1;
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueVec2(FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict)
{
    // init output.
    out.clear();

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)        return -1;

        else if (dataType == "Vec2")        {
                                                out.push_back(rtval.maybeGetMember("x").getFloat32());
                                                out.push_back(rtval.maybeGetMember("y").getFloat32());
                                            }
        else
            return -1;
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueVec3(FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict)
{
    // init output.
    out.clear();

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)        return -1;

        else if (dataType == "Vec3")        {
                                                out.push_back(rtval.maybeGetMember("x").getFloat32());
                                                out.push_back(rtval.maybeGetMember("y").getFloat32());
                                                out.push_back(rtval.maybeGetMember("z").getFloat32());
                                            }
        else
            return -1;
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueVec4(FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict)
{
    // init output.
    out.clear();

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)        return -1;

        else if (dataType == "Vec4")        {
                                                out.push_back(rtval.maybeGetMember("x").getFloat32());
                                                out.push_back(rtval.maybeGetMember("y").getFloat32());
                                                out.push_back(rtval.maybeGetMember("z").getFloat32());
                                                out.push_back(rtval.maybeGetMember("t").getFloat32());
                                            }
        else
            return -1;
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueQuat(FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict)
{
    // init output.
    out.clear();

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)        return -1;

        else if (dataType == "Quat")        {
                                                FabricCore::RTVal v;
                                                v = rtval.maybeGetMember("v");
                                                out.push_back(v.    maybeGetMember("x").getFloat32());
                                                out.push_back(v.    maybeGetMember("y").getFloat32());
                                                out.push_back(v.    maybeGetMember("z").getFloat32());
                                                out.push_back(rtval.maybeGetMember("w").getFloat32());
                                            }
        else
            return -1;
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return -4;
    }

    // done.
    return 0;
}

int BaseInterface::GetPortValueMat44(FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict)
{
    // init output.
    out.clear();

    // invalid port?
    if (!port.isValid())
        return -2;

    // set out from port value.
    try
    {
        std::string dataType = port.getDataType();
        FabricCore::RTVal rtval = port.getRTVal();

        if      (dataType.length() == 0)        return -1;

        else if (dataType == "Mat44")       {
                                                char member[32];
                                                FabricCore::RTVal rtRow;
                                                for (int i=0;i<4;i++)
                                                {
                                                    sprintf(member, "row%ld", i);
                                                    rtRow = rtval.maybeGetMember(member);
                                                    out.push_back(rtRow.maybeGetMember("x").getFloat32());
                                                    out.push_back(rtRow.maybeGetMember("y").getFloat32());
                                                    out.push_back(rtRow.maybeGetMember("z").getFloat32());
                                                    out.push_back(rtRow.maybeGetMember("t").getFloat32());
                                                }
                                            }
        else
            return -1;
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
        return -4;
    }

    // done.
    return 0;
}

void BaseInterface::SetValueOfPortBoolean(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const bool val)
{
    try
    {
        FabricCore::RTVal rtval;
        rtval = FabricCore::RTVal::ConstructBoolean(client, val);
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::SetValueOfPortSInt(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const long val)
{
    try
    {
        FabricCore::RTVal rtval;
        if      (port.getDataType() == "SInt8")   rtval = FabricCore::RTVal::ConstructSInt8 (client, val);
        else if (port.getDataType() == "SInt16")  rtval = FabricCore::RTVal::ConstructSInt16(client, val);
        else if (port.getDataType() == "SInt32")  rtval = FabricCore::RTVal::ConstructSInt32(client, val);
        else if (port.getDataType() == "SInt64")  rtval = FabricCore::RTVal::ConstructSInt64(client, val);
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::SetValueOfPortUInt(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const unsigned long val)
{
    try
    {
        FabricCore::RTVal rtval;
        if      (port.getDataType() == "UInt8")   rtval = FabricCore::RTVal::ConstructUInt8 (client, val);
        else if (port.getDataType() == "UInt16")  rtval = FabricCore::RTVal::ConstructUInt16(client, val);
        else if (port.getDataType() == "UInt32")  rtval = FabricCore::RTVal::ConstructUInt32(client, val);
        else if (port.getDataType() == "UInt64")  rtval = FabricCore::RTVal::ConstructUInt64(client, val);
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::SetValueOfPortFloat(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const double val)
{
    try
    {
        FabricCore::RTVal rtval;
        if      (port.getDataType() == "Float32") rtval = FabricCore::RTVal::ConstructFloat32(client, val);
        else if (port.getDataType() == "Float64") rtval = FabricCore::RTVal::ConstructFloat64(client, val);
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::SetValueOfPortString(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::string &val)
{
    try
    {
        FabricCore::RTVal rtval;
        rtval = FabricCore::RTVal::ConstructString(client, val.c_str());
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::SetValueOfPortVec2(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val)
{
    try
    {
        const int  N        =     2;
        const char name[16] = "Vec2";
        FabricCore::RTVal rtval;
        FabricCore::RTVal v[N];
        const bool valIsValid  = (val.size() >= N);
        for (int i=0;i<N;i++)
            v[i] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[i] : 0);
        rtval  = FabricCore::RTVal::Construct(client, name, N, v);
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::SetValueOfPortVec3(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val)
{
    try
    {
        const int  N        =     3;
        const char name[16] = "Vec3";
        FabricCore::RTVal rtval;
        FabricCore::RTVal v[N];
        const bool valIsValid  = (val.size() >= N);
        for (int i=0;i<N;i++)
            v[i] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[i] : 0);
        rtval  = FabricCore::RTVal::Construct(client, name, N, v);
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::SetValueOfPortVec4(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val)
{
    try
    {
        const int  N        =     4;
        const char name[16] = "Vec4";
        FabricCore::RTVal rtval;
        FabricCore::RTVal v[N];
        const bool valIsValid  = (val.size() >= N);
        for (int i=0;i<N;i++)
            v[i] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[i] : 0);
        rtval  = FabricCore::RTVal::Construct(client, name, N, v);
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::SetValueOfPortQuat(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val)
{
    try
    {
        FabricCore::RTVal rtval;
        FabricCore::RTVal xyz[3], v[2];
        const bool valIsValid = (val.size() >= 4);
        xyz[0] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[0] : 0);
        xyz[1] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[1] : 0);
        xyz[2] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[2] : 0);
        v[0]   = FabricCore::RTVal::Construct(client, "Vec3", 3, xyz);
        v[1]   = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[3] : 0);
        rtval  = FabricCore::RTVal::Construct(client, "Quat", 2, v);
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}

void BaseInterface::SetValueOfPortMat44(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val)
{
    try
    {
        FabricCore::RTVal rtval;
        FabricCore::RTVal xyzt[4], v[4];
        const bool valIsValid = (val.size() >= 16);
        for (int i=0;i<4;i++)
        {
            int offset = i * 4;
            xyzt[0] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[offset + 0] : 0);
            xyzt[1] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[offset + 1] : 0);
            xyzt[2] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[offset + 2] : 0);
            xyzt[3] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[offset + 3] : 0);
            v[i]    = FabricCore::RTVal::Construct(client, "Vec4", 4, xyzt);
        }
        rtval = FabricCore::RTVal::Construct(client, "Mat44", 4, v);
        binding.setArgValue(port.getName().c_str(), rtval);
    }
    catch(FabricCore::Exception e)
    {
        logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
}



