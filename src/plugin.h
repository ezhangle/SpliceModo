//
#ifndef SRC_PLUGIN_H_
#define SRC_PLUGIN_H_

// plugin version.
#define FABRICMODO_PLUGIN_VERSION   0.910

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
#define SERVER_NAME_JSONValue       "dfgJSONValue"      // the JSON custom type (used by the channels "FabricJSON").
#define SERVER_NAME_CanvasIM        "CanvasIM"          // Canvas "item modifier".
#define SERVER_NAME_CanvasPI        "CanvasPI"          // Canvas "procedural item".

// constants: fixed channels' names & co.
#define CHN_NAME_INSTOBJ            "instObj"           // out: (CanvasPI only) objref channel.
#define CHN_NAME_IO_FabricActive    "FabricActive"      // io:  enable/disable execution of DFG for this item.
#define CHN_NAME_IO_FabricEval      "FabricEval"        // io:  internal counter used to re-evaluate the item.
#define CHN_NAME_IO_FabricJSON      "FabricJSON"        // io:  custom value for persistence (read/write BaseInterface's JSON). See notes below.
#define CHN_FabricJSON_NUM          128                 // amount of FabricJSON channels. Note: modifying this value might break older lxo files!
#define CHN_FabricJSON_MAX_BYTES    ((uint32_t)64000)   // max amount of bytes per FabricJSON channel.

/*
                - notes about the "FabricJSON" channels -
  (CHN_NAME_IO_FabricJSON, CHN_FabricJSON_NUM and CHN_FabricJSON_MAX_BYTES)

  Modo has the following two limitations which make persistence a bit trickier:
    - the data of an item's channel is limited to 2^16 bytes when reading a scene file.
    - the schematic view doesn't support items with more than 256 channels.
    - there is no callback when a scene is being saved (or about to be saved).

  Persistence is therefore implemented as following:

  - rather than storing the entire JSON string in a single channel it is split
    into chunks of CHN_FabricJSON_MAX_BYTES bytes and divided amongst all the
    CHN_NAME_IO_FabricJSON channels.

  KNOWN LIMITATION:

  - if a JSON string is larger than CHN_FabricJSON_MAX_BYTES * CHN_FabricJSON_NUM
    bytes then the scene is not correctly saved.

*/

// forward declaration: log system.
#define LOG_SYSTEM_NAME "Fabric"
void feLog(void *userData, const char *s, unsigned int length);
void feLog(void *userData, const std::string &s);
void feLog(const std::string &s);
void feLogError(void *userData, const char *s, unsigned int length);
void feLogError(void *userData, const std::string &s);
void feLogError(const std::string &s);
void feLogDebug(void *userData, const char *s, unsigned int length);
void feLogDebug(void *userData, const std::string &s);
void feLogDebug(const std::string &s);
void feLogDebug(const std::string &s, int number);

#endif  // SRC_PLUGIN_H_

