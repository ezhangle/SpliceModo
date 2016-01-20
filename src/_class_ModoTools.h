#ifndef SRC__CLASS_MODOTOOLS_H_
#define SRC__CLASS_MODOTOOLS_H_

#ifdef _WIN32
  // disable some annoying VS warnings.
  #pragma warning(disable : 4530)    // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc.
  #pragma warning(disable : 4800)    // forcing value to bool 'true' or 'false'.
  #pragma warning(disable : 4806)    // unsafe operation: no value of type 'bool' promoted to type ...etc.
#endif

// includes (Modo).
#include "lxidef.h"

#include <lx_action.hpp>
#include "lx_chanmod.hpp"
#include <lx_channelui.hpp>
#include <lx_draw.hpp>
#include "lx_item.hpp"
#include "lx_io.hpp"
#include <lx_locator.hpp>
#include "lx_package.hpp"
#include "lx_plugin.hpp"
#include <lx_surface.hpp>
#include <lx_scripts.hpp>
#include <lx_tableau.hpp>
#include <lx_undo.hpp>
#include "lx_value.hpp"
#include <lx_vertex.hpp>
#include <lx_vmodel.hpp>
#include <lx_wrap.hpp>

#include <lxu_attrdesc.hpp>
#include "lxu_command.hpp"
#include "lxu_log.hpp"
#include <lxu_math.hpp>
#include <lxu_message.hpp>
#include "lxu_modifier.hpp"
#include "lxu_select.hpp"

#include "lxlog.h"
#include <lxvmath.h>

#include "lxw_command.hpp"
#include "lxw_customview.hpp"
#include "lxw_undo.hpp"

class ModoTools
{
 public:
  // item user channel structure.
  struct UsrChnDef
  {
    int  chan_index;            // item channel index.
    int  eval_index;            // evaluation index.
    std::string chan_name;      // name of the channnel.
    bool isSingleton;           // true: all other "is*" flags are equal false.
    bool isVec2x;               // true: this is the first channel of a 2D vector.
    bool isVec3x;               // true: this is the first channel of a 3D vector.
    bool isRGBr;                // true: this is the first channel of a RGB color.
    bool isRGBAr;               // true: this is the first channel of a RGBA color.
    UsrChnDef () : chan_index(-1), eval_index(-1), chan_name(""),
                   isSingleton(true),
                   isVec2x(false), isVec3x(false), isRGBr(false), isRGBAr(false) {}
  };

 public:

  // checks if the Fabric environment variables are set and outputs errors in the event log if one or more env. variables are missing.
  // params:  showMsgbox    true: display a message box informing the user about the missing env. variables.
   static void checkFabricEnvVariables(bool showMsgbox);

  // executes a command string.
  // params:  command                 command to execute, e.g. "channel.create bla item:myItem username:blabla".
  //          io_result               the result/output of the command or "" if the command does not return a value.
  //          out_err                 contains an error description if the function returns false.
  // returns: true on success, false otherwise.
  static bool ExecuteCommand(const std::string &command, std::string &io_result, std::string &out_err);

  // executes a command.
  // params:  cmdName                 the command name, e.g. "FabricCanvasConnect".
  //          args                    the arguments.
  //          io_result               the result/output of the command or "" if the command does not return a value.
  //          out_err                 contains an error description if the function returns false.
  // returns: true on success, false otherwise.
  static bool ExecuteCommand(const std::string &cmdName, const std::vector<std::string> &args, std::string &io_result, std::string &out_err);

  // fills the array io_usrChan with all usable user channels of the input item.
  // note: all members of UsrChnDef are set except for eval_index which is set to -1.
  static void usrChanCollect(CLxUser_Item &item, std::vector <UsrChnDef> &io_usrChan);

  // looks for a channel with the specified name and returns its pointer (or NULL if not found).
  static UsrChnDef *usrChanGetFromName(std::string channelName, std::vector <UsrChnDef> &usrChan);

  // checks if an item has a specific channel (user or other).
  // params:  ptr_CLxUser_Item                        pointer at CLxUser_Item (or ILxUnknownID, see parameter interpretate_ptr_as_ILxUnknownID).
  //          channelName                             name of channel.
  //          out_err                                 contains an error description if the function returns false.
  //          out_actualChannelName                   if the channel was found then this contains the "actual" channel name, typically
  //                                                  it is equal channelName, but for vector or color channels it can differ.
  //          out_isUserChannel                       true: the channel is a user channel, false: it is a fixed channel.
  //          interpretate_ptr_as_ILxUnknownID        if true then the parameter ptr_CLxUser_Item is not interpretated
  //                                                  as being a pointer at CLxUser_Item but as being a ILxUnknownID.
  // returns: true if the item ptr_CLxUser_Item has a channel called channelName, false otherwise.
  static bool HasChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_actualChannelName, std::string &out_err, bool &out_isUserChannel, bool interpretate_ptr_as_ILxUnknownID);
  static bool HasChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err, bool &out_isUserChannel, bool interpretate_ptr_as_ILxUnknownID);

  // creates a new user channel.
  // params:  ptr_CLxUser_Item        pointer at CLxUser_Item.
  //          channelName             name of new channel.
  //          dataType                channel's data type (e.g. "float", "integer", "string").
  //          structType              structure type, may be "" (e.g. "scalar", "vecXY", "vecXYZ", "vecRGB", "vecRGBA").
  //          out_err                 contains an error description if the function returns false.
  // returns: true on success, false otherwise.
  static bool CreateUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, const std::string &dataType, const std::string &structType, std::string &out_err);

  // returns true if an item with a given name exists in the scene.
  // params   itemName    the name of an item (it can be the user name or the ident, both work).
  // returns: true if item exists.
  static bool ItemExists(const std::string &itemName);
  static bool ItemExists(const char *itemName);

  // gets an item based on the item name.
  // params   itemName    the name of an item (it can be the user name or the ident, both work).
  //          out_item    the item.
  // returns: true if the item was found and out_item successfully set.
  static bool GetItem(const std::string &itemName, CLxUser_Item  &out_item);
  static bool GetItem(const char *itemName, CLxUser_Item &out_item);

  // gets the type of an item.
  // params   itemName        the name of an item (it can be the user name or the ident, both work).
  //          out_typeName    the type of the item or "" if the item was not found.
  // returns: true if the item was found and out_typeName set..
  static bool GetItemType(const std::string &itemName, std::string &out_typeName);
  static bool GetItemType(const char *itemName, std::string &out_typeName);

  // gets all user channels of an item.
  // params:  ptr_CLxUser_Item        pointer at CLxUser_Item.
  //          out_usrChannels         output: array of user channel names.
  //          out_err                 contains an error description if the function returns false.
  // returns: on success the amount of user channels (0 or more), on failure -1.
  static int GetUserChannels(void *ptr_CLxUser_Item, std::vector <std::string> &out_usrChannels, std::string &out_err);

  // params:  ptr_CLxUser_Item                        pointer at CLxUser_Item (or ILxUnknownID, see parameter interpretate_ptr_as_ILxUnknownID).
  //          channelName                             name of channel.
  //          out_err                                 contains an error description if the function returns false.
  //          interpretate_ptr_as_ILxUnknownID        if true then the parameter ptr_CLxUser_Item is not interpretated
  //                                                  as being a pointer at CLxUser_Item but as being a ILxUnknownID.
  // returns: true on success, false otherwise.
  static bool DeleteUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, std::string &out_err, bool interpretate_ptr_as_ILxUnknownID = false);

  // deletes all user channels.
  // params:  ptr_CLxUser_Item        pointer at CLxUser_Item.
  //          out_err                 contains an error description if the function returns false.
  // returns: true on success, false otherwise.
  static bool DeleteAllUserChannels(void *ptr_CLxUser_Item, std::string &out_err);

  // renames an existing user channel.
  // params:  ptr_CLxUser_Item                        pointer at CLxUser_Item.
  //          channelName                             current name of channel.
  //          channelNameNew                          new name of channel.
  //          out_err                                 contains an error description if the function returns false.
  //          interpretate_ptr_as_ILxUnknownID        if true then the parameter ptr_CLxUser_Item is not interpretated
  //                                                  as being a pointer at CLxUser_Item but as being a ILxUnknownID.
  // returns: true on success, false otherwise.
  static bool RenameUserChannel(void *ptr_CLxUser_Item, const std::string &channelName, const std::string &channelNameNew, std::string &out_err, bool interpretate_ptr_as_ILxUnknownID = false);

  // gets the value of a channel as a boolean, integer, string, etc.
  // params:  attr        attribute with channels.
  //          eval_index  channel index.
  //          out         will contain the result.
  //          strict      true: the type must match perfectly, false: the type must 'kind of' match and will be converted if necessary (and if possible).
  // returns: 0 on success, -1 wrong port type, -2 invalid port, -3 otherwise.
  static int GetChannelValueAsBoolean   (CLxUser_Attributes &attr, int eval_index, bool                 &out, bool strict = false);
  static int GetChannelValueAsInteger   (CLxUser_Attributes &attr, int eval_index, int                  &out, bool strict = false);
  static int GetChannelValueAsFloat     (CLxUser_Attributes &attr, int eval_index, double               &out, bool strict = false);
  static int GetChannelValueAsString    (CLxUser_Attributes &attr, int eval_index, std::string          &out, bool strict = false);
  static int GetChannelValueAsVector2   (CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict = false);
  static int GetChannelValueAsVector3   (CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict = false);
  static int GetChannelValueAsColor     (CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict = false);
  static int GetChannelValueAsRGB       (CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict = false);
  static int GetChannelValueAsRGBA      (CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict = false);
  static int GetChannelValueAsQuaternion(CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict = false);
  static int GetChannelValueAsMatrix44  (CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict = false);
  static int GetChannelValueAsXfo       (CLxUser_Attributes &attr, int eval_index, std::vector <double> &out, bool strict = false);

  // invalidates an item so that it gets re-evaluated:
  // this is done by calling the command "FabricCanvasIncEval" which will increase
  // the value of the internal integer channel called "FabricEval" by 1.
  static void InvalidateItem(ILxUnknownID item_obj);

  // clears Modo's undo stack.
  static void ClearUndoStack();
};

#endif  // SRC__CLASS_MODOTOOLS_H_
