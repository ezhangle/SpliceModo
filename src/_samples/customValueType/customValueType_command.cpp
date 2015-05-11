/*
 *
 *  Custom Value Type.
 *
 *  This sample plugin demonstrates how to create a custom value type, and how
 *  to evaluate the custom value type as part of a modifier. The value type can
 *  be stored in the scene file, however, the stored value is markedly different
 *  from the evaluated value.
 *
 */

#include "customValueType.hpp"

/*
 *  Disambiguate with a namespace.
 */

namespace Command
{

/*
 *  Implement a simple command. This command sets the value stored in the action
 *  for the custom channel on the selected item. It should also have things like
 *  messages and enable state, but it's just an example of a way the data can be
 *  set in the action.
 */

class Command : public CLxBasicCommand
{
  public:
    static void initialize ()
    {
      CLxGenericPolymorph    *srv;

      srv = new CLxPolymorph            <Command>;
      srv->AddInterface    (new CLxIfc_Command    <Command>);
      srv->AddInterface    (new CLxIfc_Attributes    <Command>);
      srv->AddInterface    (new CLxIfc_AttributesUI  <Command>);
      srv->AddInterface    (new CLxIfc_StaticDesc    <Command>);

      lx::AddServer      (COMMAND_SERVER_NAME, srv);
    }

    Command ();
    
    int     basic_CmdFlags    ()        LXx_OVERRIDE;
    void     cmd_Execute    (unsigned int flags)    LXx_OVERRIDE;
  
    static LXtTagInfoDesc   descInfo[];
};

Command::Command ()
{
  /*
   *  A single integer argument is used to set the value stored in the
   *  custom channel.
   */
  
  dyna_Add (ARG_VALUE, LXsTYPE_INTEGER);
}

int Command::basic_CmdFlags()
{
  /*
   *  The command changes the scene state and should be undoable.
   */

  return LXfCMD_MODEL | LXfCMD_UNDO;
}

void Command::cmd_Execute (unsigned int flags)
{
  /*
   *  The command gets the selected item and gets the custom channel value
   *  for writing. It then sets the custom value using the integer argument
   *  passed to the command.
   */

  CLxUser_Message    &message = basic_Message();
  CLxUser_ChannelWrite   chan_write;
  CLxUser_Item     item;
  CLxUser_Value     value;
  
  CLxItemSelection   item_sel;
  int       arg_value = 0;
  
  if (item_sel.GetFirst (item) && item.test ())
  {
    /*
     *  We only want to operate on our custom item type.
     */
  
    if (!item.IsA (gItemType.Type ()))
      return;
      
    /*
     *  Get the argument value, if it's set. Although, in reality,
     *  we shouldn't get to this point if it's not set.
     */
     
    if (dyna_IsSet (0))
      arg_value = dyna_Int (0);
    
    /*
     *  Get our custom value as a writeable value object.
     */
     
    if (chan_write.from (item, 0.0))
    {
      if (chan_write.Object (item, CHAN_CUSTOMVALUE, value) && value.test ())
      {
        /*
         *  Call SetInt on the Value object to set the
         *  value.
         */
      
        value.Set (arg_value);
        
        return;
      }
    }
  }
  
  message.SetCode (LXe_FAILED);
}

LXtTagInfoDesc Command::descInfo[] =
{
  { 0 }
};

void initialize ()
{
  Command::initialize ();
}

};  // End Namespace.