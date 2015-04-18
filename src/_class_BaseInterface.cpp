#include "plugin.h"

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
  m_id                     = s_maxId++;
  m_ILxUnknownID_dfgModoIM = NULL;
  m_ILxUnknownID_dfgModoPI = NULL;

  // construct the client
  if (s_instances.size() == 0)
  {
    try
    {
      // create a client
      FabricCore::Client::CreateOptions options;
      memset( &options, 0, sizeof( options ) );
      options.optimizationType = FabricCore::ClientOptimizationType_Background;
      s_client = FabricCore::Client(&logFunc, NULL, &options);

      // load basic extensions
      s_client.loadExtension("Math",     "", false);
      s_client.loadExtension("Geometry", "", false);
      s_client.loadExtension("FileIO",   "", false);

      // create a host for Canvas
      s_host = new DFGWrapper::Host(s_client);

      // create KL AST manager
      s_manager = new ASTWrapper::KLASTManager(&s_client);
      s_manager->loadAllExtensionsFromExtsPath();
    }
    catch (FabricCore::Exception e)
    {
      logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    }
  }

  // instert in map.
  s_instances.insert(std::pair<unsigned int, BaseInterface*>(m_id, this));

  //
  try
  {
    // create an empty binding
    m_binding = s_host->createBindingToNewGraph();
    m_binding.setNotificationCallback(bindingNotificationCallback, this);

    // set the graph on the view
    DFGWrapper::View::setGraph(DFGWrapper::GraphExecutablePtr::StaticCast(m_binding.getExecutable()));
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

BaseInterface::~BaseInterface()
{
  std::map<unsigned int, BaseInterface*>::iterator it = s_instances.find(m_id);

  m_binding = DFGWrapper::Binding();

  if (it != s_instances.end())
  {
    s_instances.erase(it);
    if (s_instances.size() == 0)
    {
      try
      {
        printf("Destructing client...\n");
        s_stack.clear();
        delete(s_manager);
        delete(s_host);
        s_client = FabricCore::Client();
      }
      catch (FabricCore::Exception e)
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

BaseInterface *BaseInterface::getFromId(unsigned int id)
{
  std::map<unsigned int, BaseInterface*>::iterator it = s_instances.find(id);
  if (it == s_instances.end())
    return NULL;
  return it->second;
}

FabricCore::Client *BaseInterface::getClient()
{
  return &s_client;
}

FabricServices::DFGWrapper::Host *BaseInterface::getHost()
{
  return s_host;
}

FabricServices::DFGWrapper::Binding *BaseInterface::getBinding()
{
  return &m_binding;
}

FabricServices::ASTWrapper::KLASTManager *BaseInterface::getManager()
{
  return s_manager;
}

FabricServices::Commands::CommandStack *BaseInterface::getStack()
{
  return &s_stack;
}

std::string BaseInterface::getJSON()
{
  try
  {
    return m_binding.exportJSON();
  }
  catch (FabricCore::Exception e)
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
    m_binding.setNotificationCallback(bindingNotificationCallback, this);
    DFGWrapper::View::setGraph(DFGWrapper::GraphExecutablePtr::StaticCast(m_binding.getExecutable()));
  }
  catch (FabricCore::Exception e)
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

void BaseInterface::onPortRenamed(FabricServices::DFGWrapper::PortPtr port, const char *oldName)
{
  if (port.isNull())
  {
    std::string s = "BaseInterface::onPortRenamed(): port == NULL";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  if (!port->isValid())
  {
    std::string s = "BaseInterface::onPortRenamed(): port->isValid() == false";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  if (!oldName)
  {
    std::string s = "BaseInterface::onPortRenamed(): oldName == NULL";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    std::string err;
    CLxUser_Item item;

    if      (m_ILxUnknownID_dfgModoIM)  item.set((ILxUnknownID)m_ILxUnknownID_dfgModoIM);
    else if (m_ILxUnknownID_dfgModoPI)  item.set((ILxUnknownID)m_ILxUnknownID_dfgModoPI);
    else                              { err = "BaseInterface::onPortRenamed(): m_ILxUnknownID_dfgModo??? == NULL";
                                        logErrorFunc(0, err.c_str(), err.length());
                                        return; }

    if (!item.test()) { err = "BaseInterface::onPortRenamed(): item((ILxUnknownID)m_ILxUnknownID_dfgModo???) failed";
                        logErrorFunc(0, err.c_str(), err.length());
                        return; }

    if (!ModoTools::RenameUserChannel(&item, std::string(oldName), port->getName(), err))
      logErrorFunc(0, err.c_str(), err.length());

    // workaround: there currently is no way to tell Modo (via SceneItemListener) that a port was renamed,
    // so we simply create a dummy port and delete it right away.
    std::string dummyName = oldName;
    dummyName += "tmpWorkaround127";
    if (!ModoTools::CreateUserChannel(&item, dummyName, "float", "", err))
    {
      logErrorFunc(0, err.c_str(), err.length());
      return;
    }
    if (!ModoTools::DeleteUserChannel(&item, dummyName, err))
    {
      logErrorFunc(0, err.c_str(), err.length());
      return;
    }
  }
  catch (FabricCore::Exception e)
  {
    std::string s = std::string("BaseInterface::onPortRenamed(): ") + (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");;
    logErrorFunc(NULL, s.c_str(), s.length());
  }
}

void BaseInterface::bindingNotificationCallback(void *userData, char const *jsonCString, uint32_t jsonLength)
{
  // check pointers.
  if (!userData || !jsonCString)
    return;

  // go.
  try
  {
    // get the base interface, its graph and the notification variant.
    BaseInterface                                  &b            = *(BaseInterface *)userData;
    FabricServices::DFGWrapper::GraphExecutablePtr  graph        = DFGWrapper::GraphExecutablePtr::StaticCast(b.getBinding()->getExecutable());
    FabricCore::Variant                             notification = FabricCore::Variant::CreateFromJSON(jsonCString, jsonLength);

    // get the notification's description and possibly the value of name.
    const FabricCore::Variant *vDesc = notification.getDictValue("desc");
    const FabricCore::Variant *vName = notification.getDictValue("name");
    if (!vDesc)   return;
    std::string nDesc = vDesc->getStringData();
    std::string nName = (vName ? vName->getStringData() : "");

    // get the Modo item's ILxUnknownID.
    void             *unknownID = NULL;
    if (!unknownID)   unknownID = b.m_ILxUnknownID_dfgModoIM;
    if (!unknownID)   unknownID = b.m_ILxUnknownID_dfgModoPI;

    // handle notification.
    std::string err = "";
    {
      if      (nDesc == "argTypeChanged")
      {
        // re-create Modo user channel.
        if (unknownID)
        {
          ModoTools::DeleteUserChannel(unknownID, nName, err, true);
          if (!graph.isNull())
            b.CreateModoUserChannelForPort(graph->getPort(nName.c_str()));
        }
      }

      else if (nDesc == "argRemoved")
      {
        // delete the Modo user channel.
        if (unknownID)
          ModoTools::DeleteUserChannel(unknownID, nName, err, true);
      }

      else
      {
        //logFunc(NULL, nDesc.c_str(), nDesc.length());
      }
    }
  }
  catch (FabricCore::Exception e)
  {
    std::string s = std::string("BaseInterface::bindingNotificationCallback(): ") + (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");;
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }
}

void BaseInterface::logFunc(void *userData, const char *message, unsigned int length)
{
  if (s_logFunc)
  {
    if (message)
      s_logFunc(userData, message, length);
  }
  else
  {
    printf("BaseInterface: %s\n", message);
  }
}

void BaseInterface::logErrorFunc(void *userData, const char *message, unsigned int length)
{
  if (s_logErrorFunc)
  {
    if (message)
      s_logErrorFunc(userData, message, length);
  }
  else
  {
    printf("BaseInterface: error: %s\n", message);
  }
}

bool BaseInterface::HasPort(const char *in_portName, const bool testForInput)
{
  // check/init.
  if (!in_portName || in_portName[0] == '\0')  return false;
  const FabricCore::DFGPortType portType = (testForInput ? FabricCore::DFGPortType_In : FabricCore::DFGPortType_Out);

  // set the port name we will use for the search, i.e. check if the last two characters resemble
  // something like ".X", ".R", etc. meaning we are dealing with a scalar channel belonging to a vector/color channel.
  std::string portName = in_portName;
  if (portName.length() > 2)
  {
    char lastChar       = portName.back();   portName.pop_back();
    char beforeLastChar = portName.back();   portName.pop_back();
    if (beforeLastChar  != '.')
      portName = in_portName;
  }

  // go.
  try
  {
    // get the graph.
    FabricServices::DFGWrapper::GraphExecutablePtr graph = DFGWrapper::GraphExecutablePtr::StaticCast(m_binding.getExecutable());
    if (graph.isNull())
    {
      std::string s = "BaseInterface::HasPort(): m_binding.getExecutable() returned NULL pointer.";
      logErrorFunc(NULL, s.c_str(), s.length());
      return false;
    }

    // get the port.
    FabricServices::DFGWrapper::PortPtr port = graph->getPort(portName.c_str());

    // return result.
    return (   !port.isNull()
            &&  port->isValid()
            &&  port->getPortType() == portType);
  }
  catch (FabricCore::Exception e)
  {
    std::string s = std::string("BaseInterface::HasPort(): ") + (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    logErrorFunc(NULL, s.c_str(), s.length());
    return false;
  }
}

bool BaseInterface::HasInputPort(const char *portName)
{
  return HasPort(portName, true);
}

bool BaseInterface::HasInputPort(const std::string &portName)
{
  return HasPort(portName.c_str(), true);
}

bool BaseInterface::HasOutputPort(const char *portName)
{
  return HasPort(portName, false);
}

bool BaseInterface::HasOutputPort(const std::string &portName)
{
  return HasPort(portName.c_str(), false);
}

int BaseInterface::GetPortValueBoolean(FabricServices::DFGWrapper::PortPtr port, bool &out, bool strict)
{
  // init output.
  out = false;

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)    return -1;

    else if (resolvedType == "Boolean")     out = rtval.getBoolean();

    else if (!strict)
    {
      if      (resolvedType == "Float32")   out = (0 != rtval.getFloat32());
      else if (resolvedType == "Float64")   out = (0 != rtval.getFloat64());

      else if (resolvedType == "SInt8")     out = (0 != rtval.getSInt8());
      else if (resolvedType == "SInt16")    out = (0 != rtval.getSInt16());
      else if (resolvedType == "SInt32")    out = (0 != rtval.getSInt32());
      else if (resolvedType == "SInt64")    out = (0 != rtval.getSInt64());

      else if (resolvedType == "UInt8")     out = (0 != rtval.getUInt8());
      else if (resolvedType == "UInt16")    out = (0 != rtval.getUInt16());
      else if (resolvedType == "UInt32")    out = (0 != rtval.getUInt32());
      else if (resolvedType == "UInt64")    out = (0 != rtval.getUInt64());

      else return -1;
    }
    else return -1;
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueInteger(FabricServices::DFGWrapper::PortPtr port, int &out, bool strict)
{
  // init output.
  out = 0;

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)    return -1;

    else if (resolvedType == "SInt8")       out = (int)rtval.getSInt8();
    else if (resolvedType == "SInt16")      out = (int)rtval.getSInt16();
    else if (resolvedType == "SInt32")      out = (int)rtval.getSInt32();
    else if (resolvedType == "SInt64")      out = (int)rtval.getSInt64();

    else if (resolvedType == "UInt8")       out = (int)rtval.getUInt8();
    else if (resolvedType == "UInt16")      out = (int)rtval.getUInt16();
    else if (resolvedType == "UInt32")      out = (int)rtval.getUInt32();
    else if (resolvedType == "UInt64")      out = (int)rtval.getUInt64();

    else if (!strict)
    {
      if      (resolvedType == "Boolean")   out = (int)rtval.getBoolean();

      else if (resolvedType == "Float32")   out = (int)rtval.getFloat32();
      else if (resolvedType == "Float64")   out = (int)rtval.getFloat64();

      else return -1;
    }
    else return -1;
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueFloat(FabricServices::DFGWrapper::PortPtr port, double &out, bool strict)
{
  // init output.
  out = 0;

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)    return -1;

    else if (resolvedType == "Float32")     out = (double)rtval.getFloat32();
    else if (resolvedType == "Float64")     out = (double)rtval.getFloat64();

    else if (!strict)
    {
      if      (resolvedType == "Boolean")   out = (double)rtval.getBoolean();

      else if (resolvedType == "SInt8")     out = (double)rtval.getSInt8();
      else if (resolvedType == "SInt16")    out = (double)rtval.getSInt16();
      else if (resolvedType == "SInt32")    out = (double)rtval.getSInt32();
      else if (resolvedType == "SInt64")    out = (double)rtval.getSInt64();

      else if (resolvedType == "UInt8")     out = (double)rtval.getUInt8();
      else if (resolvedType == "UInt16")    out = (double)rtval.getUInt16();
      else if (resolvedType == "UInt32")    out = (double)rtval.getUInt32();
      else if (resolvedType == "UInt64")    out = (double)rtval.getUInt64();

      else return -1;
    }
    else return -1;
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueString(FabricServices::DFGWrapper::PortPtr port, std::string &out, bool strict)
{
  // init output.
  out = "";

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)    return -1;

    else if (resolvedType == "String")      out = rtval.getStringCString();

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
        snprintf(s, sizeof(s), "%ld", i);
        out = s;
        return 0;
      }

      if (GetPortValueFloat(port, f, true) == 0)
      {
        snprintf(s, sizeof(s), "%f", f);
        out = s;
        return 0;
      }

      return -1;
    }
    else
      return -1;
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueVec2(FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)      return -1;

    else if (resolvedType == "Vec2")        {
                                              out.push_back(rtval.maybeGetMember("x").getFloat32());
                                              out.push_back(rtval.maybeGetMember("y").getFloat32());
                                            }
    else
      return -1;
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueVec3(FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)      return -1;

    else if (resolvedType == "Vec3")        {
                                              out.push_back(rtval.maybeGetMember("x").getFloat32());
                                              out.push_back(rtval.maybeGetMember("y").getFloat32());
                                              out.push_back(rtval.maybeGetMember("z").getFloat32());
                                            }
    else if (!strict)
    {
        if      (resolvedType == "Color")   {
                                              out.push_back(rtval.maybeGetMember("r").getFloat32());
                                              out.push_back(rtval.maybeGetMember("g").getFloat32());
                                              out.push_back(rtval.maybeGetMember("b").getFloat32());
                                            }
        else if (resolvedType == "Vec4")    {
                                              out.push_back(rtval.maybeGetMember("x").getFloat32());
                                              out.push_back(rtval.maybeGetMember("y").getFloat32());
                                              out.push_back(rtval.maybeGetMember("z").getFloat32());
                                            }
        else
          return -1;
    }
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueVec4(FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)      return -1;

    else if (resolvedType == "Vec4")        {
                                              out.push_back(rtval.maybeGetMember("x").getFloat32());
                                              out.push_back(rtval.maybeGetMember("y").getFloat32());
                                              out.push_back(rtval.maybeGetMember("z").getFloat32());
                                              out.push_back(rtval.maybeGetMember("t").getFloat32());
                                            }
    else if (!strict)
    {
        if      (resolvedType == "Color")   {
                                              out.push_back(rtval.maybeGetMember("r").getFloat32());
                                              out.push_back(rtval.maybeGetMember("g").getFloat32());
                                              out.push_back(rtval.maybeGetMember("b").getFloat32());
                                              out.push_back(rtval.maybeGetMember("a").getFloat32());
                                            }
        else
          return -1;
    }
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueColor(FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)      return -1;

    else if (resolvedType == "Color")      {
                                              out.push_back(rtval.maybeGetMember("r").getFloat32());
                                              out.push_back(rtval.maybeGetMember("g").getFloat32());
                                              out.push_back(rtval.maybeGetMember("b").getFloat32());
                                              out.push_back(rtval.maybeGetMember("a").getFloat32());
                                           }
    else if (!strict)
    {
        if      (resolvedType == "Vec4")   {
                                              out.push_back(rtval.maybeGetMember("x").getFloat32());
                                              out.push_back(rtval.maybeGetMember("y").getFloat32());
                                              out.push_back(rtval.maybeGetMember("z").getFloat32());
                                              out.push_back(rtval.maybeGetMember("t").getFloat32());
                                            }
        else if (resolvedType == "RGB")     {
                                              out.push_back(rtval.maybeGetMember("r").getUInt8() / 255.0);
                                              out.push_back(rtval.maybeGetMember("g").getUInt8() / 255.0);
                                              out.push_back(rtval.maybeGetMember("b").getUInt8() / 255.0);
                                              out.push_back(1);
                                            }
        else if (resolvedType == "RGBA")    {
                                              out.push_back(rtval.maybeGetMember("r").getUInt8() / 255.0);
                                              out.push_back(rtval.maybeGetMember("g").getUInt8() / 255.0);
                                              out.push_back(rtval.maybeGetMember("b").getUInt8() / 255.0);
                                              out.push_back(rtval.maybeGetMember("a").getUInt8() / 255.0);
                                            }
        else
          return -1;
    }
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueRGB(FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)    return -1;

    else if (resolvedType == "RGB")       {
                                            out.push_back(rtval.maybeGetMember("r").getUInt8() / 255.0);
                                            out.push_back(rtval.maybeGetMember("g").getUInt8() / 255.0);
                                            out.push_back(rtval.maybeGetMember("b").getUInt8() / 255.0);
                                          }
    else if (!strict)
    {
      if      (resolvedType == "RGBA")    {
                                            out.push_back(rtval.maybeGetMember("r").getUInt8() / 255.0);
                                            out.push_back(rtval.maybeGetMember("g").getUInt8() / 255.0);
                                            out.push_back(rtval.maybeGetMember("b").getUInt8() / 255.0);
                                          }
      else if (resolvedType == "Color")   {
                                            out.push_back(rtval.maybeGetMember("r").getFloat32());
                                            out.push_back(rtval.maybeGetMember("g").getFloat32());
                                            out.push_back(rtval.maybeGetMember("b").getFloat32());
                                          }
      else
        return -1;
    }
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueRGBA(FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
      std::string resolvedType = port->getResolvedType();
      FabricCore::RTVal rtval  = port->getArgValue();

      if      (resolvedType.length() == 0)      return -1;

      else if (resolvedType == "RGBA")        {
                                                out.push_back(rtval.maybeGetMember("r").getUInt8() / 255.0);
                                                out.push_back(rtval.maybeGetMember("g").getUInt8() / 255.0);
                                                out.push_back(rtval.maybeGetMember("b").getUInt8() / 255.0);
                                                out.push_back(rtval.maybeGetMember("a").getUInt8() / 255.0);
                                              }
      else if (!strict)
      {
          if      (resolvedType == "RGB")     {
                                                out.push_back(rtval.maybeGetMember("r").getUInt8() / 255.0);
                                                out.push_back(rtval.maybeGetMember("g").getUInt8() / 255.0);
                                                out.push_back(rtval.maybeGetMember("b").getUInt8() / 255.0);
                                                out.push_back(1);
                                              }
          else if (resolvedType == "Color")   {
                                                out.push_back(rtval.maybeGetMember("r").getFloat32());
                                                out.push_back(rtval.maybeGetMember("g").getFloat32());
                                                out.push_back(rtval.maybeGetMember("b").getFloat32());
                                                out.push_back(rtval.maybeGetMember("a").getFloat32());
                                              }
          else
            return -1;
      }
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueQuat(FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)      return -1;

    else if (resolvedType == "Quat")        {
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
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValueMat44(FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict)
{
  // init output.
  out.clear();

  // invalid port?
  if (port.isNull() || !port->isValid())
    return -2;

  // set out from port value.
  try
  {
    std::string resolvedType = port->getResolvedType();
    FabricCore::RTVal rtval  = port->getArgValue();

    if      (resolvedType.length() == 0)      return -1;

    else if (resolvedType == "Mat44")       {
                                              char member[32];
                                              FabricCore::RTVal rtRow;
                                              for (int i = 0; i < 4; i++)
                                              {
                                                snprintf(member, sizeof(member), "row%ld", i);
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
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return -4;
  }

  // done.
  return 0;
}

int BaseInterface::GetPortValuePolygonMesh(FabricServices::DFGWrapper::PortPtr  port,
                                           unsigned int                        &out_numVertices,
                                           unsigned int                        &out_numPolygons,
                                           unsigned int                        &out_numSamples,
                                           std::vector <float>                 *out_positions,
                                           std::vector <uint32_t>              *out_polygonNumVertices,
                                           std::vector <uint32_t>              *out_polygonVertices,
                                           std::vector <float>                 *out_polygonNodeNormals,
                                           bool                                 strict)
{
  // init output.
  out_numVertices = 0;
  out_numPolygons = 0;
  out_numSamples  = 0;
  if (out_positions)          out_positions          -> clear();
  if (out_polygonNumVertices) out_polygonNumVertices -> clear();
  if (out_polygonVertices)    out_polygonVertices    -> clear();
  if (out_polygonNodeNormals) out_polygonNodeNormals -> clear();

  // invalid port?
  if (port.isNull() || !port->isValid())
      return -2;

  // set out from port value.
  int errID = 0;
  do
  {
    try
    {
      // check type.
      std::string resolvedType = port->getResolvedType();
      if (   resolvedType.length() == 0
          || resolvedType != "PolygonMesh")
        return -1;

      // RTVal of the polygon mesh.
      FabricCore::RTVal rtMesh = port->getArgValue();

      // get amount of points, polys, etc.
      out_numVertices = rtMesh.callMethod("UInt64", "pointCount",         0, 0).getUInt64();
      out_numPolygons = rtMesh.callMethod("UInt64", "polygonCount",       0, 0).getUInt64();
      out_numSamples  = rtMesh.callMethod("UInt64", "polygonPointsCount", 0, 0).getUInt64();

      // get vertex positions.
      if (   out_positions  != NULL
          && out_numVertices > 0  )
      {
        std::vector <float> &data = *out_positions;

        // resize output array(s).
            data.   resize(3 * out_numVertices);
        if (data.size() != 3 * out_numVertices)
        { errID = -3;
          break;  }

        // fill output array(s).
        std::vector <FabricCore::RTVal> args(2);
        args[0] = FabricCore::RTVal::ConstructExternalArray(*getClient(), "Float32", data.size(), (void *)data.data());
        args[1] = FabricCore::RTVal::ConstructUInt32(*getClient(), 3);
        rtMesh.callMethod("", "getPointsAsExternalArray", 2, &args[0]);
      }

      // get polygonal description.
      if (  (   out_polygonNumVertices != NULL
             || out_polygonVertices    != NULL)
          && out_numPolygons            > 0
          && out_numSamples             > 0  )
      {
        std::vector <uint32_t> tmpNum;
        std::vector <uint32_t> tmpIdx;
        std::vector <uint32_t> &dataNum = (out_polygonNumVertices ? *out_polygonNumVertices : tmpNum);
        std::vector <uint32_t> &dataIdx = (out_polygonVertices    ? *out_polygonVertices    : tmpIdx);

        // resize output array(s).
            dataNum.   resize(out_numPolygons);
        if (dataNum.size() != out_numPolygons)
        { errID = -3;
          break;  }
            dataIdx.   resize(out_numSamples);
        if (dataIdx.size() != out_numSamples)
        { errID = -3;
          break;  }

        // fill output array(s).
        std::vector <FabricCore::RTVal> args(2);
        args[0] = FabricCore::RTVal::ConstructExternalArray(*getClient(), "UInt32", dataNum.size(), (void *)dataNum.data());
        args[1] = FabricCore::RTVal::ConstructExternalArray(*getClient(), "UInt32", dataIdx.size(), (void *)dataIdx.data());
        rtMesh.callMethod("", "getTopologyAsCountsIndicesExternalArrays", 2, &args[0]);
      }

      // get polygon node normals.
      if (   out_polygonNodeNormals != NULL
          && out_numPolygons         > 0
          && out_numSamples          > 0  )
      {
        std::vector <float> &data = *out_polygonNodeNormals;

        // resize output array(s).
            data.   resize(3 * out_numSamples);
        if (data.size() != 3 * out_numSamples)
        { errID = -3;
          break;  }

        // fill output array(s).
        std::vector <FabricCore::RTVal> args(1);
        args[0] = FabricCore::RTVal::ConstructExternalArray(*getClient(), "Float32", data.size(), (void *)data.data());
        rtMesh.callMethod("", "getNormalsAsExternalArray", 1, &args[0]);
      }
    }
    catch (FabricCore::Exception e)
    {
      logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
      errID = -4;
      break;
    }
  } while (false);

  // if an error occurred then clear all outputs.
  if (errID)
  {
    if (errID == -3)
    { std::string s = "memory error: failed to resize std::vector<>";
      logErrorFunc(NULL, s.c_str(), s.length());  }
    out_numVertices = 0;
    out_numPolygons = 0;
    out_numSamples  = 0;
    if (out_positions)          out_positions          -> clear();
    if (out_polygonNumVertices) out_polygonNumVertices -> clear();
    if (out_polygonVertices)    out_polygonVertices    -> clear();
    if (out_polygonNodeNormals) out_polygonNodeNormals -> clear();
  }

  // done.
  return errID;
}

void BaseInterface::SetValueOfPortBoolean(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const bool val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortBoolean(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    FabricCore::RTVal rtval;
    rtval = FabricCore::RTVal::ConstructBoolean(client, val);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortSInt(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const int32_t val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortSInt(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    FabricCore::RTVal rtval;
    std::string resolvedType = port->getResolvedType();
    if      (resolvedType == "SInt8")   rtval = FabricCore::RTVal::ConstructSInt8 (client, val);
    else if (resolvedType == "SInt16")  rtval = FabricCore::RTVal::ConstructSInt16(client, val);
    else if (resolvedType == "SInt32")  rtval = FabricCore::RTVal::ConstructSInt32(client, val);
    else if (resolvedType == "SInt64")  rtval = FabricCore::RTVal::ConstructSInt64(client, val);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortUInt(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const uint32_t val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortUInt(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    FabricCore::RTVal rtval;
    std::string resolvedType = port->getResolvedType();
    if      (resolvedType == "UInt8")   rtval = FabricCore::RTVal::ConstructUInt8 (client, val);
    else if (resolvedType == "UInt16")  rtval = FabricCore::RTVal::ConstructUInt16(client, val);
    else if (resolvedType == "UInt32")  rtval = FabricCore::RTVal::ConstructUInt32(client, val);
    else if (resolvedType == "UInt64")  rtval = FabricCore::RTVal::ConstructUInt64(client, val);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortFloat(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const double val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortFloat(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    FabricCore::RTVal rtval;
    std::string resolvedType = port->getResolvedType();
    if      (resolvedType == "Float32") rtval = FabricCore::RTVal::ConstructFloat32(client, val);
    else if (resolvedType == "Float64") rtval = FabricCore::RTVal::ConstructFloat64(client, val);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortString(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::string &val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortString(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    FabricCore::RTVal rtval;
    rtval = FabricCore::RTVal::ConstructString(client, val.c_str());
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortVec2(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortVec2(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    const int  N        =     2;
    const char name[16] = "Vec2";
    FabricCore::RTVal rtval;
    FabricCore::RTVal v[N];
    const bool valIsValid  = (val.size() >= N);
    for (int i = 0; i < N; i++)
      v[i] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[i] : 0);
    rtval  = FabricCore::RTVal::Construct(client, name, N, v);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortVec3(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortVec3(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    const int  N        =     3;
    const char name[16] = "Vec3";
    FabricCore::RTVal rtval;
    FabricCore::RTVal v[N];
    const bool valIsValid  = (val.size() >= N);
    for (int i = 0; i < N; i++)
      v[i] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[i] : 0);
    rtval  = FabricCore::RTVal::Construct(client, name, N, v);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortVec4(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortVec4(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    const int  N        =     4;
    const char name[16] = "Vec4";
    FabricCore::RTVal rtval;
    FabricCore::RTVal v[N];
    const bool valIsValid  = (val.size() >= N);
    for (int i = 0; i < N; i++)
      v[i] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[i] : 0);
    rtval  = FabricCore::RTVal::Construct(client, name, N, v);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortColor(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortColor(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    const int  N        =      4;
    const char name[16] = "Color";
    FabricCore::RTVal rtval;
    FabricCore::RTVal v[N];
    const bool valIsValid  = (val.size() >= N);
    for (int i = 0; i < N; i++)
      v[i] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[i] : 0);
    rtval  = FabricCore::RTVal::Construct(client, name, N, v);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortRGB(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortRGB(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    const int  N        =    3;
    const char name[16] = "RGB";
    FabricCore::RTVal rtval;
    FabricCore::RTVal v[N];
    const bool valIsValid  = (val.size() >= N);
    for (int i = 0; i < N; i++)
      v[i] = FabricCore::RTVal::ConstructUInt8(client, valIsValid ? (uint8_t)__max(0.0, __min(255.0, 255.0 * val[i])) : 0);
    rtval  = FabricCore::RTVal::Construct(client, name, N, v);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortRGBA(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortRGBA(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    const int  N        =     4;
    const char name[16] = "RGBA";
    FabricCore::RTVal rtval;
    FabricCore::RTVal v[N];
    const bool valIsValid  = (val.size() >= N);
    for (int i = 0; i < N; i++)
      v[i] = FabricCore::RTVal::ConstructUInt8(client, valIsValid ? (uint8_t)__max(0.0, __min(255.0, 255.0 * val[i])) : 0);
    rtval  = FabricCore::RTVal::Construct(client, name, N, v);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortQuat(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortQuat(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

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
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

void BaseInterface::SetValueOfPortMat44(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::SetValueOfPortMat44(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return;
  }

  try
  {
    FabricCore::RTVal rtval;
    FabricCore::RTVal xyzt[4], v[4];
    const bool valIsValid = (val.size() >= 16);
    for (int i = 0; i < 4; i++)
    {
      int offset = i * 4;
      xyzt[0] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[offset + 0] : 0);
      xyzt[1] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[offset + 1] : 0);
      xyzt[2] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[offset + 2] : 0);
      xyzt[3] = FabricCore::RTVal::ConstructFloat32(client, valIsValid ? val[offset + 3] : 0);
      v[i]    = FabricCore::RTVal::Construct(client, "Vec4", 4, xyzt);
    }
    rtval = FabricCore::RTVal::Construct(client, "Mat44", 4, v);
    binding.setArgValue(port->getName(), rtval);
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
  }
}

bool BaseInterface::CreateModoUserChannelForPort(FabricServices::DFGWrapper::PortPtr port)
{
  if (port.isNull() || !port->isValid())
  {
    std::string s = "BaseInterface::CreateModoUserChannelForPort(): port no good (either NULL or invalid).";
    logErrorFunc(NULL, s.c_str(), s.length());
    return false;
  }

  try
  {
    std::string err;
    CLxUser_Item item;

    if      (m_ILxUnknownID_dfgModoIM)  item.set((ILxUnknownID)m_ILxUnknownID_dfgModoIM);
    else if (m_ILxUnknownID_dfgModoPI)  item.set((ILxUnknownID)m_ILxUnknownID_dfgModoPI);
    else                        {   err = "m_ILxUnknownID_dfgModo??? == NULL";
                                    logErrorFunc(0, err.c_str(), err.length());
                                    return false;   }

    if (!item.test())   {   err = "item((ILxUnknownID)m_ILxUnknownID_dfgModo???) failed";
                            logErrorFunc(0, err.c_str(), err.length());
                            return false;    }

    std::string resolvedType = port->getResolvedType();
    std::string structType   = "";

    if      (   resolvedType == "")         {   err = "port->getResolvedType() == \"\"";
                                                logErrorFunc(0, err.c_str(), err.length());
                                                return false;    }

    else if (   resolvedType == "Boolean")  {   resolvedType = "boolean";                           }

    else if (   resolvedType == "Integer"
             || resolvedType == "SInt8"
             || resolvedType == "SInt16"
             || resolvedType == "SInt32"
             || resolvedType == "SInt64"
             || resolvedType == "UInt8"
             || resolvedType == "UInt16"
             || resolvedType == "UInt32"
             || resolvedType == "UInt64")   {   resolvedType = "integer";                           }

    else if (   resolvedType == "Scalar"
             || resolvedType == "Float32"
             || resolvedType == "Float64")  {   resolvedType = "float";                             }

    else if (   resolvedType == "String")   {   resolvedType = "string";                            }

    else if (   resolvedType == "Quat")     {   resolvedType = "quaternion";                        }

    else if (   resolvedType == "Mat44")    {   resolvedType = "matrix";                            }

    else if (   resolvedType == "Vec2")     {   resolvedType = "float";     structType = "vecXY";   }
    else if (   resolvedType == "Vec3")     {   resolvedType = "float";     structType = "vecXYZ";  }

    else if (   resolvedType == "Color")    {   resolvedType = "float";     structType = "vecRGBA"; }
    else if (   resolvedType == "RGB")      {   resolvedType = "float";     structType = "vecRGB";  }
    else if (   resolvedType == "RGBA")     {   resolvedType = "float";     structType = "vecRGBA"; }

    else
    {
      err = "unable to create user channel, data type \"" + resolvedType + "\" not yet implemented";
      logErrorFunc(0, err.c_str(), err.length());
      return false;
    }

    if (!ModoTools::CreateUserChannel(&item, port->getName(), resolvedType, structType, err))
      logErrorFunc(0, err.c_str(), err.length());
  }
  catch (FabricCore::Exception e)
  {
    logErrorFunc(NULL, e.getDesc_cstr(), e.getDescLength());
    return false;
  }

  // done.
  return true;
}



