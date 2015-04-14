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

        void *m_item_obj_dfgModoIM;     // pointer at ILxUnknownID (for the item modifier node "dfgModoIM").
        void *m_item_obj_dfgModoPI;     // pointer at ILxUnknownID (for the item modifier node "dfgModoPI").

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
        static FabricServices::ASTWrapper::KLASTManager * getManager();
        static FabricServices::Commands::CommandStack * getStack();

        // persistence
        std::string getJSON();
        void setFromJSON(const std::string & json);

        // logging.
        static void setLogFunc(void (*in_logFunc)(void *, const char *, unsigned int));
        static void setLogErrorFunc(void (*in_logErrorFunc)(void *, const char *, unsigned int));

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
        virtual void onPortRenamed(FabricServices::DFGWrapper::Port port, const char * oldName);
        virtual void onPinRenamed(FabricServices::DFGWrapper::Pin pin, const char * oldName) {}
        virtual void onExecMetadataChanged(FabricServices::DFGWrapper::Executable exec, const char * key, const char * metadata) {}
        virtual void onExtDepAdded(const char * extension, const char * version) {}
        virtual void onNodeCacheRuleChanged(const char * path, const char * rule) {}
        virtual void onExecCacheRuleChanged(const char * path, const char * rule) {}

    private:
        static void logFunc(void * userData, const char * message, unsigned int length);
        static void logErrorFunc(void * userData, const char * message, unsigned int length);
        static void (*s_logFunc)(void *, const char *, unsigned int);
        static void (*s_logErrorFunc)(void *, const char *, unsigned int);

        static FabricCore::Client s_client;
        static FabricServices::DFGWrapper::Host * s_host;
        FabricServices::DFGWrapper::Binding m_binding;
        static FabricServices::ASTWrapper::KLASTManager * s_manager;
        static FabricServices::Commands::CommandStack s_stack;
        unsigned int m_id;
        static unsigned int s_maxId;
        static std::map<unsigned int, BaseInterface*> s_instances;

    private:
        // returns true if the graph has a port called portName that matches the port type (input/output).
        // params:  portName        name of the port.
        //          testForInput    true: look for input port, else for output port.
        bool HasPort(const char *portName, const bool testForInput);

    public:
        // returns true if the graph has an input port called portName.
        bool HasInputPort(const char *portName);
        bool HasInputPort(const std::string &portName);

        // returns true if the graph has an output port called portName.
        bool HasOutputPort(const char *portName);
        bool HasOutputPort(const std::string &portName);

        // gets the value of a port.
        // params:  port        the port.
        //          out         will contain the result.
        //          strict      true: the type must match perfectly, false: the type must 'kind of' match and will be converted if necessary (and if possible).
        // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 unknown, -4 Fabric exception.
        static int GetPortValueBoolean(FabricServices::DFGWrapper::Port &port, bool                 &out, bool strict = false);
        static int GetPortValueInteger(FabricServices::DFGWrapper::Port &port, int                  &out, bool strict = false);
        static int GetPortValueFloat  (FabricServices::DFGWrapper::Port &port, double               &out, bool strict = false);
        static int GetPortValueString (FabricServices::DFGWrapper::Port &port, std::string          &out, bool strict = false);
        static int GetPortValueVec2   (FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict = false);
        static int GetPortValueVec3   (FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict = false);
        static int GetPortValueVec4   (FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict = false);
        static int GetPortValueColor  (FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict = false);
        static int GetPortValueRGB    (FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict = false);
        static int GetPortValueRGBA   (FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict = false);
        static int GetPortValueQuat   (FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict = false);
        static int GetPortValueMat44  (FabricServices::DFGWrapper::Port &port, std::vector <double> &out, bool strict = false);

        // gets the value of a "PolygonMesh" port.
        // params:  port        the port.
        //          out_*       will contain the result. These may be NULL. See parameters for more information.
        //          strict      true: the type must match perfectly, false: the type must 'kind of' match and will be converted if necessary (and if possible).
        // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 unknown, -4 Fabric exception.
        static int GetPortValuePolygonMesh( FabricServices::DFGWrapper::Port    &port,
                                            unsigned int                        &out_numVertices,   
                                            unsigned int                        &out_numPolygons,
                                            unsigned int                        &out_numSamples,
                                            std::vector <double>                *out_positions,     // vertex 3D positions (as a flat array).
                                            std::vector <double>                *out_positions      = NULL,     // vertex 3D positions (as a flat array).
                                            bool                                 strict             = false);

        // sets the value of a port.
        static void SetValueOfPortBoolean(FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const bool                  val);
        static void SetValueOfPortSInt   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const long                  val);
        static void SetValueOfPortUInt   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const unsigned long         val);
        static void SetValueOfPortFloat  (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const double                val);
        static void SetValueOfPortString (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::string          &val);
        static void SetValueOfPortVec2   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val);
        static void SetValueOfPortVec3   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val);
        static void SetValueOfPortVec4   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val);
        static void SetValueOfPortColor  (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val);
        static void SetValueOfPortRGB    (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val);
        static void SetValueOfPortRGBA   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val);
        static void SetValueOfPortQuat   (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val);
        static void SetValueOfPortMat44  (FabricCore::Client &client, FabricServices::DFGWrapper::Binding &binding, FabricServices::DFGWrapper::Port &port, const std::vector <double> &val);

        // creates a Modo user channel for a Fabric port.
        // returns: true on success, false otherwise.
        bool CreateModoUserChannelForPort(FabricServices::DFGWrapper::Port &port);
};

#endif




