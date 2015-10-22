//
#ifndef SRC_PLUGIN_H_
#define SRC_PLUGIN_H_

// plugin version.
#define FABRICMODO_PLUGIN_VERSION   0.110   // note: version 0.021 was "FMX Demo", i.e. the last version before using Matt Cox' surfaceItem sample instead of my own emReader code.

// disable some annoying VS warnings.
#ifdef _WIN32
  #pragma warning(disable : 4530)   // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc.
  #pragma warning(disable : 4800)   // forcing value to bool 'true' or 'false'.
  #pragma warning(disable : 4806)   // unsafe operation: no value of type 'bool' promoted to type ...etc.
#endif

// includes.
#include <string>
#include <FTL/AutoSet.h>

// win32 stuff.
#ifdef _WIN32
  #define snprintf  _snprintf
#endif

// constants: Modo item/server names.
#define SERVER_NAME_JSONValue       "dfgJSONValue"    // the JSON custom type (used by the channels "FabricJSON").
#define SERVER_NAME_CanvasIM        "CanvasIM"        // Canvas "item modifier".
#define SERVER_NAME_CanvasPI        "CanvasPI"        // Canvas "procedural item".
#define SERVER_NAME_CanvasPIpilot   "CanvasPIpilot"   // Canvas "procedural item" (old version based on emReader).

// constants: fixed channel names.
#define CHN_NAME_INSTOBJ          "instObj"       // out: (CanvasPI only) objref channel.
#define CHN_NAME_IO_FabricActive  "FabricActive"  // io:  enable/disable execution of DFG for this item.
#define CHN_NAME_IO_FabricEval    "FabricEval"    // io:  internal counter used to re-evaluate the item.
#define CHN_NAME_IO_FabricJSON    "FabricJSON"    // io:  custom value for persistence (read/write BaseInterface's JSON).

// forward declaration: log system.
#define LOG_SYSTEM_NAME "Fabric"
void feLog(void *userData, const char *s, unsigned int length);
void feLog(void *userData, const std::string &s);
void feLog(const std::string &s);
void feLogError(void *userData, const char *s, unsigned int length);
void feLogError(void *userData, const std::string &s);
void feLogError(const std::string &s);

#endif  // SRC_PLUGIN_H_

