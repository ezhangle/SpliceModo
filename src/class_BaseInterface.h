#ifndef __BASEINTERFACE_H__
#define __BASEINTERFACE_H__

#include <DFGWrapper/DFGWrapper.h>
#include <map>

// a management class for client and host
class BaseInterface : public FabricServices::DFGWrapper::View
{
public:
  BaseInterface();
  ~BaseInterface();

  // instance management
  // right now there are no locks in place,
  // assuming that the DCC will only access
  // these things from the main thread.
  unsigned int getId();
  static BaseInterface * getFromId(unsigned int id);

  // accessors
  static FabricCore::Client * getClient();
  static FabricServices::DFGWrapper::Host * getHost();
  FabricServices::DFGWrapper::Binding * getBinding();

  // persistence
  std::string getJSON();
  void setFromJSON(const std::string & json);

  // notifications
  // for now we only implement onPortInserted and onPortRemoved
  virtual void onNotification(char const * json) {}
  virtual void onNodeInserted(FabricServices::DFGWrapper::Node node) {}
  virtual void onNodeRemoved(FabricServices::DFGWrapper::Node node) {}
  virtual void onPinInserted(FabricServices::DFGWrapper::Pin pin) {}
  virtual void onPinRemoved(FabricServices::DFGWrapper::Pin pin) {}
  virtual void onPortInserted(FabricServices::DFGWrapper::Port port);
  virtual void onPortRemoved(FabricServices::DFGWrapper::Port port);
  virtual void onEndPointsConnected(FabricServices::DFGWrapper::Port src, FabricServices::DFGWrapper::Port dst) {}
  virtual void onEndPointsDisconnected(FabricServices::DFGWrapper::Port src, FabricServices::DFGWrapper::Port dst) {}
  virtual void onNodeMetadataChanged(FabricServices::DFGWrapper::Node node, const char * key, const char * metadata) {}
  virtual void onNodeTitleChanged(FabricServices::DFGWrapper::Node node, const char * title) {}
  virtual void onPortRenamed(FabricServices::DFGWrapper::Port port, const char * oldName) {}
  virtual void onPinRenamed(FabricServices::DFGWrapper::Pin pin, const char * oldName) {}
  virtual void onExecMetadataChanged(FabricServices::DFGWrapper::Executable exec, const char * key, const char * metadata) {}
  virtual void onExtDepAdded(const char * extension, const char * version) {}
  virtual void onNodeCacheRuleChanged(const char * path, const char * rule) {}
  virtual void onExecCacheRuleChanged(const char * path, const char * rule) {}

private:

  static void logFunc(void * userData, const char * message, unsigned int length);

  static FabricCore::Client s_client;
  static FabricServices::DFGWrapper::Host * s_host;
  FabricServices::DFGWrapper::Binding m_binding;
  unsigned int m_id;
  static unsigned int s_maxId;
  static std::map<unsigned int, BaseInterface*> s_instances;
};

#endif
