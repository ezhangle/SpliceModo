// includes.
#include "plugin.h"

// log system.
CItemLog gLog;
void feLog(void *userData, const char *s, unsigned int length)
{
    gLog.Message(LXe_INFO, "[FABRIC]", s, " ");
    FabricUI::DFG::DFGLogWidget::log(s);
}
void feLog(void *userData, const std::string &s)
{
    feLog(userData, s.c_str(), s.length());
}
void feLogError(void *userData, const char *s, unsigned int length)
{
    gLog.Message(LXe_FAILED, "[FABRIC ERROR]", s, " ");
    std::string t = s;
    t = "Error: " + t;
    FabricUI::DFG::DFGLogWidget::log(t.c_str());
}
void feLogError(void *userData, const std::string &s)
{
    feLogError(userData, s.c_str(), s.length());
}

// plugin initialization.
void initialize()
{
    // Fabric.
    {
        // set log function pointers.
        BaseInterface::setLogFunc(feLog);
        BaseInterface::setLogErrorFunc(feLogError);
    }

    // Modo.
    {
        dfgExportJSON     :: Command::  initialize();
        dfgImportJSON     :: Command::  initialize();
        dfgLogVersion     :: Command::  initialize();
        dfgOpenCanvas     :: Command::  initialize();
        dfgStoreInChannel :: Command::  initialize();
        dfgModoIM::                     initialize();
        dfgModoPI::                     initialize();
    }
 }

// plugin clean up.
void cleanup()
{
}

