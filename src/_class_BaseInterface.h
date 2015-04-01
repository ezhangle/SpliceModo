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

        // pointer at ILxUnknownID (for the item modifier node "dfgModoIM").
        void *m_item_obj_dfgModoIM;

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

    public:
        // returns true if the graph has an input port called portName.
        bool HasInputPort(const char *portName);
        bool HasInputPort(const std::string &portName);

        // returns true if the graph has an output port called portName.
        bool HasOutputPort(const char *portName);
        bool HasOutputPort(const std::string &portName);

        // gets the value of a port as an integer.
        // params:  port        the port.
        //          out         will contain the result or 0 if an error occurred.
        //          strict      true: the type must match perfectly, false: the type must 'kind of' match and is converted if necessary.
        // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 otherwise.
        static int GetPortValueAsInteger(FabricServices::DFGWrapper::Port &port, int &out, bool strict = false);

        // gets the value of a port as a float.
        // params:  port        the port.
        //          out         will contain the result or 0 if an error occurred.
        //          strict      true: the type must match perfectly, false: the type must 'kind of' match and is converted if necessary.
        // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 otherwise.
        static int GetPortValueAsFloat(FabricServices::DFGWrapper::Port &port, double &out, bool strict = false);

        // gets the value of a port as a string.
        // params:  port        the port.
        //          out         will contain the result or "" if an error occurred.
        //          strict      true: the type must match perfectly, false: the type must 'kind of' match and is converted if necessary.
        // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 otherwise.
        static int GetPortValueAsString(FabricServices::DFGWrapper::Port &port, std::string &out, bool strict = false);
};

#endif




