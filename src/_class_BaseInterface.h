#ifndef __BASEINTERFACE_H__
#define __BASEINTERFACE_H__

// disable some annoying VS warnings.
#pragma warning (disable : 4530)  // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc.
#pragma warning (disable : 4800)  // forcing value to bool 'true' or 'false'.
#pragma warning (disable : 4806)  // unsafe operation: no value of type 'bool' promoted to type ...etc.

// includes.
#include <DFGWrapper/DFGWrapper.h>
#include <ASTWrapper/KLASTManager.h>
#include <Commands/CommandStack.h>
#include <map>

// a management class for client and host
class BaseInterface : public FabricServices::DFGWrapper::View
{
    public:
        BaseInterface();
        ~BaseInterface();

        void *m_ILxUnknownID_dfgModoIM;     // ILxUnknownID of the Modo item modifier node   "dfgModoIM"). Cast this to ILxUnknownID.
        void *m_ILxUnknownID_dfgModoPI;     // ILxUnknownID of the Modo procedural item node "dfgModoPI"). Cast this to ILxUnknownID.

        // instance management
        // right now there are no locks in place,
        // assuming that the DCC will only access
        // these things from the main thread.
        unsigned int getId();
        static BaseInterface *getFromId(unsigned int id);

        // accessors
        static FabricCore::Client                       *getClient();
        static FabricServices::DFGWrapper::Host         *getHost();
        FabricServices::DFGWrapper::Binding             *getBinding();
        static FabricServices::ASTWrapper::KLASTManager *getManager();
        static FabricServices::Commands::CommandStack   *getStack();

        // persistence
        std::string getJSON();
        void setFromJSON(const std::string & json);

        // logging.
        static void setLogFunc(void (*in_logFunc)(void *, const char *, unsigned int));
        static void setLogErrorFunc(void (*in_logErrorFunc)(void *, const char *, unsigned int));

        // notifications
        virtual void onNotification(char const * json) {}
        virtual void onNodeInserted(FabricServices::DFGWrapper::NodePtr node) {}
        virtual void onNodeRemoved(FabricServices::DFGWrapper::NodePtr node) {}
        virtual void onPinInserted(FabricServices::DFGWrapper::PinPtr pin) {}
        virtual void onPinRemoved(FabricServices::DFGWrapper::PinPtr pin) {}
        virtual void onPortInserted(FabricServices::DFGWrapper::PortPtr port) {}
        virtual void onPortRemoved(FabricServices::DFGWrapper::PortPtr port);
        virtual void onEndPointsConnected(FabricServices::DFGWrapper::EndPointPtr src, FabricServices::DFGWrapper::EndPointPtr dst) {}
        virtual void onEndPointsDisconnected(FabricServices::DFGWrapper::EndPointPtr src, FabricServices::DFGWrapper::EndPointPtr dst) {}
        virtual void onNodeMetadataChanged(FabricServices::DFGWrapper::NodePtr node, const char * key, const char * metadata) {}
        virtual void onNodeTitleChanged(FabricServices::DFGWrapper::NodePtr node, const char * title) {}
        virtual void onPortRenamed(FabricServices::DFGWrapper::PortPtr port, const char * oldName);
        virtual void onPinRenamed(FabricServices::DFGWrapper::PinPtr pin, const char * oldName) {}
        virtual void onExecMetadataChanged(FabricServices::DFGWrapper::ExecutablePtr exec, const char * key, const char * metadata) {}
        virtual void onExtDepAdded(const char * extension, const char * version) {}
        virtual void onExtDepRemoved(const char * extension, const char * version) {}
        virtual void onNodeCacheRuleChanged(const char * path, const char * rule) {}
        virtual void onExecCacheRuleChanged(const char * path, const char * rule) {}
        virtual void onPortResolvedTypeChanged(FabricServices::DFGWrapper::PortPtr port, const char * resolvedType) {}
        virtual void onPinResolvedTypeChanged(FabricServices::DFGWrapper::PinPtr pin, const char * resolvedType) {}

    private:

        // binding notifications.
        static void bindingNotificationCallback(void *userData, char const *jsonCString, uint32_t jsonLength);

        // logging.
        static void logFunc(void *userData, const char *message, unsigned int length);
        static void logErrorFunc(void * userData, const char * message, unsigned int length);
        static void (*s_logFunc)(void *, const char *, unsigned int);
        static void (*s_logErrorFunc)(void *, const char *, unsigned int);

        // member vars.
        unsigned int        m_id;
        static unsigned int s_maxId;
        static FabricCore::Client                        s_client;
        static FabricServices::DFGWrapper::Host         *s_host;
        static FabricServices::ASTWrapper::KLASTManager *s_manager;
        static FabricServices::Commands::CommandStack    s_stack;
        FabricServices::DFGWrapper::Binding              m_binding;
        static std::map<unsigned int, BaseInterface*>    s_instances;

    private:
        // returns true if the binding's executable has a port called portName that matches the port type (input/output).
        // params:  in_portName     name of the port.
        //          testForInput    true: look for input port, else for output port.
        bool HasPort(const char *in_portName, const bool testForInput);

    public:
        // returns true if the binding's executable has an input port called portName.
        bool HasInputPort(const char *portName);
        bool HasInputPort(const std::string &portName);

        // returns true if the binding's executable has an output port called portName.
        bool HasOutputPort(const char *portName);
        bool HasOutputPort(const std::string &portName);

        // gets the value of a port.
        // params:  port        the port.
        //          out         will contain the result.
        //          strict      true: the type must match perfectly, false: the type must 'kind of' match and will be converted if necessary (and if possible).
        // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 unknown, -4 Fabric exception.
        static int GetPortValueBoolean(FabricServices::DFGWrapper::PortPtr port, bool                 &out, bool strict = false);
        static int GetPortValueInteger(FabricServices::DFGWrapper::PortPtr port, int                  &out, bool strict = false);
        static int GetPortValueFloat  (FabricServices::DFGWrapper::PortPtr port, double               &out, bool strict = false);
        static int GetPortValueString (FabricServices::DFGWrapper::PortPtr port, std::string          &out, bool strict = false);
        static int GetPortValueVec2   (FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict = false);
        static int GetPortValueVec3   (FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict = false);
        static int GetPortValueVec4   (FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict = false);
        static int GetPortValueColor  (FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict = false);
        static int GetPortValueRGB    (FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict = false);
        static int GetPortValueRGBA   (FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict = false);
        static int GetPortValueQuat   (FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict = false);
        static int GetPortValueMat44  (FabricServices::DFGWrapper::PortPtr port, std::vector <double> &out, bool strict = false);

        // gets the value of a "PolygonMesh" port.
        // params:  port        the port.
        //          out_*       will contain the result. These may be NULL. See parameters for more information.
        //          strict      true: the type must match perfectly, false: the type must 'kind of' match and will be converted if necessary (and if possible).
        // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 memory error, -4 Fabric exception.
        static int GetPortValuePolygonMesh( FabricServices::DFGWrapper::PortPtr  port,
                                            unsigned int                        &out_numVertices,                       // amount of vertices.
                                            unsigned int                        &out_numPolygons,                       // amount of polygons.
                                            unsigned int                        &out_numSamples,                        // amount of samples.
                                            std::vector <float>                 *out_positions              = NULL,     // vertex positions (as a flat array).
                                            std::vector <uint32_t>              *out_polygonNumVertices     = NULL,     // polygon vertex counts.
                                            std::vector <uint32_t>              *out_polygonVertices        = NULL,     // polygon vertex indices.
                                            std::vector <float>                 *out_polygonNodeNormals     = NULL,     // polygon node normals.
                                            bool                                 strict                     = false);

        // sets the value of a port.
        static void SetValueOfPortBoolean(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const bool                  val);
        static void SetValueOfPortSInt   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const long                  val);
        static void SetValueOfPortUInt   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const unsigned long         val);
        static void SetValueOfPortFloat  (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const double                val);
        static void SetValueOfPortString (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::string          &val);
        static void SetValueOfPortVec2   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val);
        static void SetValueOfPortVec3   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val);
        static void SetValueOfPortVec4   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val);
        static void SetValueOfPortColor  (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val);
        static void SetValueOfPortRGB    (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val);
        static void SetValueOfPortRGBA   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val);
        static void SetValueOfPortQuat   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val);
        static void SetValueOfPortMat44  (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::PortPtr port, const std::vector <double> &val);

        // creates a Modo user channel for a Fabric port.
        // returns: true on success, false otherwise.
        bool CreateModoUserChannelForPort(FabricServices::DFGWrapper::PortPtr port);
};

#endif




