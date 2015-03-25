#include "class_BaseInterface.h"

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
  m_id = s_maxId++;

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
  logFunc(0, "A port was inserted. We should really reflect that in the DCC.", 62);
}

void BaseInterface::onPortRemoved(FabricServices::DFGWrapper::Port port)
{
  logFunc(0, "A port was removed. We should really reflect that in the DCC.", 61);
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


