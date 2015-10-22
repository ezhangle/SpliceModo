#ifndef SRC__CLASS_BASEINTERFACE_H_
#define SRC__CLASS_BASEINTERFACE_H_

#ifdef _WIN32
  // disable some annoying VS warnings.
  #pragma warning(disable : 4530)  // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc.
  #pragma warning(disable : 4800)  // forcing value to bool 'true' or 'false'.
  #pragma warning(disable : 4806)  // unsafe operation: no value of type 'bool' promoted to type ...etc.
#endif

// includes.
#include <ASTWrapper/KLASTManager.h>
#include <map>

class DFGUICmdHandlerDCC;

// a management class for client and host
class BaseInterface
{
 public:

  BaseInterface();
  ~BaseInterface();

  void *m_ILxUnknownID_CanvasIM;       // ILxUnknownID of the Modo item modifier node   "CanvasIM").      Cast this to ILxUnknownID.
  void *m_ILxUnknownID_CanvasPI;       // ILxUnknownID of the Modo procedural item node "CanvasPI").      Cast this to ILxUnknownID.
  void *m_ILxUnknownID_CanvasPIpilot;  // ILxUnknownID of the Modo procedural item node "CanvasPIpilot"). Cast this to ILxUnknownID.

  // instance management
  // right now there are no locks in place,
  // assuming that the DCC will only access
  // these things from the main thread.
  unsigned int getId();
  static BaseInterface *getFromId(unsigned int id);

  // accessors
  static FabricCore::Client                       *getClient();
  static FabricCore::DFGHost                       getHost();
  FabricCore::DFGBinding                           getBinding();
  static FabricServices::ASTWrapper::KLASTManager *getManager();
  DFGUICmdHandlerDCC                              *getCmdHandler();

  // persistence
  std::string getJSON();
  void setFromJSON(const std::string & json);

  // logging.
  static void setLogFunc(void (*in_logFunc)(void *, const char *, unsigned int));
  static void setLogErrorFunc(void (*in_logErrorFunc)(void *, const char *, unsigned int));

  // binding notifications.
  static void bindingNotificationCallback(void *userData, char const *jsonCString, uint32_t jsonLength);

 private:

  // logging.
  static void logFunc(void *userData, const char *message, unsigned int length);
  static void logErrorFunc(void * userData, const char * message, unsigned int length);
  static void reportFunc(void *reportUserdata, FEC_ReportSource source, FEC_ReportLevel level, char const *lineCStr, uint32_t lineSize);
  static void (*s_logFunc)(void *, const char *, unsigned int);
  static void (*s_logErrorFunc)(void *, const char *, unsigned int);

  // member vars.
  unsigned int        m_id;
  static unsigned int s_maxId;
  static FabricCore::Client                        s_client;
  static FabricCore::DFGHost                       s_host;
  static FabricServices::ASTWrapper::KLASTManager *s_manager;
  FabricCore::DFGBinding                           m_binding;
  DFGUICmdHandlerDCC                              *m_cmdHandler;
  static std::map<unsigned int, BaseInterface*>    s_instances;
  bool                                             m_evaluating;
  
  // returns true if the binding's executable has a port called portName that matches the port type (input/output).
  // params:  in_portName     name of the port.
  //          testForInput    true: look for input port, else for output port.
  bool HasPort(const char *in_portName, const bool testForInput);

 public:

  // returns the amount of base interfaces.
  static int GetNumBaseInterfaces(void)  {  return s_instances.size();  }

  // gets the name of the item to which this binding belongs to.
  std::string GetItemName(void);

  // functions for (re)setting/querying the m_evaluating member.
  // note: when m_evaluating is 'true' then the bindingNotificationCallback() function returns early.
  void SetEvaluating  (void)  { m_evaluating = true;  }
  void ResetEvaluating(void)  { m_evaluating = false; }
  bool IsEvaluating   (void)  { return m_evaluating;  }

  // returns true if the binding's executable has an input port called portName.
  bool HasInputPort(const char *portName);
  bool HasInputPort(const std::string &portName);

  // returns true if the binding's executable has an output port called portName.
  bool HasOutputPort(const char *portName);
  bool HasOutputPort(const std::string &portName);

  // gets the value of an argument (= port).
  // params:  binding     ref at binding.
  //          argName     name of the argument (= port).
  //          out         will contain the result.
  //          strict      true: the type must match perfectly, false: the type must 'kind of' match and will be converted if necessary (and if possible).
  // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 unknown, -4 Fabric exception.
  static int GetArgValueBoolean(FabricCore::DFGBinding &binding, char const *argName, bool                 &out, bool strict = false);
  static int GetArgValueInteger(FabricCore::DFGBinding &binding, char const *argName, int                  &out, bool strict = false);
  static int GetArgValueFloat  (FabricCore::DFGBinding &binding, char const *argName, double               &out, bool strict = false);
  static int GetArgValueString (FabricCore::DFGBinding &binding, char const *argName, std::string          &out, bool strict = false);
  static int GetArgValueVec2   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueVec3   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueVec4   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueColor  (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueRGB    (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueRGBA   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueQuat   (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);
  static int GetArgValueMat44  (FabricCore::DFGBinding &binding, char const *argName, std::vector <double> &out, bool strict = false);

  // gets the value of a "PolygonMesh" argument (= port).
  // params:  binding     ref at binding.
  //          argName     name of the argument (= port).
  //          out_*       will contain the result. These may be NULL. See parameters for more information.
  //          strict      true: the type must match perfectly, false: the type must 'kind of' match and will be converted if necessary (and if possible).
  // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 memory error, -4 Fabric exception.
  static int GetArgValuePolygonMesh(FabricCore::DFGBinding    &binding,
                                     char const               *argName,
                                     unsigned int             &out_numVertices,                       // amount of vertices.
                                     unsigned int             &out_numPolygons,                       // amount of polygons.
                                     unsigned int             &out_numSamples,                        // amount of samples.
                                     std::vector <float>      *out_positions              = NULL,     // vertex positions (as a flat array).
                                     std::vector <uint32_t>   *out_polygonNumVertices     = NULL,     // polygon vertex counts.
                                     std::vector <uint32_t>   *out_polygonVertices        = NULL,     // polygon vertex indices.
                                     std::vector <float>      *out_polygonNodeNormals     = NULL,     // polygon node normals.
                                     bool                      strict                     = false);

  // sets the value of an argument (= port).
  static void SetValueOfArgBoolean      (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const bool                  val);
  static void SetValueOfArgSInt         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const int32_t               val);
  static void SetValueOfArgUInt         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const uint32_t              val);
  static void SetValueOfArgFloat        (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const double                val);
  static void SetValueOfArgString       (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::string          &val);
  static void SetValueOfArgVec2         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgVec3         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgVec4         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgColor        (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgRGB          (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgRGBA         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgQuat         (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);
  static void SetValueOfArgMat44        (FabricCore::Client &client, FabricCore::DFGBinding &binding, char const *argName, const std::vector <double> &val);

  // creates a Modo matching (i.e. same name, type, data type) user channel for a Fabric argument (= port).
  // returns: true on success, false otherwise.
  bool CreateModoUserChannelForPort(FabricCore::DFGBinding const &binding, char const *argName);
};

#endif  // SRC__CLASS_BASEINTERFACE_H_


