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

namespace Item
{

/*
 *  Implement the Package and Instance. The instance doesn't have to do anything,
 *  but the package adds the standard set of channels. This is about as simple
 *  as it could be.
 */

class Instance : public CLxImpl_PackageInstance
{
  public:
    static void initialize ()
    {
      CLxGenericPolymorph  *srv = NULL;

      srv = new CLxPolymorph            <Instance>;
      srv->AddInterface    (new CLxIfc_PackageInstance  <Instance>);

      lx::AddSpawner      (ITEM_SERVER_NAME".inst", srv);
    }
};

class Package : public CLxImpl_Package
{
  public:
    static void initialize ()
    {
      CLxGenericPolymorph  *srv = NULL;

      srv = new CLxPolymorph            <Package>;
      srv->AddInterface    (new CLxIfc_Package    <Package>);
      srv->AddInterface    (new CLxIfc_StaticDesc    <Package>);

      lx::AddServer      (ITEM_SERVER_NAME, srv);
    }

    Package () : _inst_spawn (ITEM_SERVER_NAME".inst") {}
  
    LxResult   pkg_SetupChannels  (ILxUnknownID addChan_obj)  LXx_OVERRIDE;
    LxResult    pkg_Attach    (void **ppvObj)      LXx_OVERRIDE;
    LxResult   pkg_TestInterface  (const LXtGUID *guid)    LXx_OVERRIDE;

    static LXtTagInfoDesc   descInfo[];
  
  private:
    CLxSpawner <Instance>   _inst_spawn;
};

LxResult Package::pkg_SetupChannels (ILxUnknownID addChan_obj)
{
  /*
   *  Add some basic built in channels. This is just our custom value type
   *  and a string channel that can be used for reading the raw value.
   */
  
  CLxUser_AddChannel   add_chan (addChan_obj);
  LxResult     result = LXe_FAILED;
  
  if (add_chan.test ())
  {
    add_chan.NewChannel (CHAN_CUSTOMVALUE, VALUE_TYPE_NAME);
    add_chan.SetStorage (VALUE_TYPE_NAME);
  
    add_chan.NewChannel (CHAN_RAWVALUE, LXsTYPE_STRING);
    add_chan.SetStorage (LXsTYPE_STRING);
    
    result = LXe_OK;
  }
  
  return result;
}

LxResult Package::pkg_Attach (void **ppvObj)
{
  /*
   *  Allocate an instance of the package instance.
   */

  _inst_spawn.Alloc (ppvObj);
  
  return ppvObj[0] ? LXe_OK : LXe_FAILED;
}

LxResult Package::pkg_TestInterface (const LXtGUID *guid)
{
  /*
   *  This is called for the various interfaces this package could
   *  potentially support, it should return a result code to indicate if
   *  it implements the specified guid.
   */

  return _inst_spawn.TestInterfaceRC (guid);
}

LXtTagInfoDesc Package::descInfo[] =
{
  { LXsPKG_SUPERTYPE, LXsITYPE_ITEMMODIFY },
  { 0 }
};

/*
 *  Implement the Modifier Element and Server. This reads the custom value type
 *  as an input and writes the raw value to the output channel.
 */

class Element : public CLxItemModifierElement
{
  public:
    Element          (CLxUser_Evaluation &eval, ILxUnknownID item_obj);
  
    void      Eval      (CLxUser_Evaluation &eval, CLxUser_Attributes &attr)  LXx_OVERRIDE;
  
  private:
    int       _chan_index;
};

class Modifier : public CLxItemModifierServer
{
  public:
    static void initialize()
    {
      CLxExport_ItemModifierServer <Modifier> (ITEM_SERVER_NAME".mod");
    }
  
    const char  *ItemType    ()              LXx_OVERRIDE;
  
    CLxItemModifierElement *Alloc    (CLxUser_Evaluation &eval, ILxUnknownID item_obj)  LXx_OVERRIDE;
};

Element::Element (CLxUser_Evaluation &eval, ILxUnknownID item_obj)
{
  /*
   *  In the constructor, we want to add the input and output channels
   *  required for this modifier. There are only two channels; our custom
   *  channel type and a string output that represents the raw value.
   */

  CLxUser_Item     item (item_obj);

  if (!item.test())
    return;

  _chan_index = eval.AddChan (item, CHAN_CUSTOMVALUE, LXfECHAN_READ);
          eval.AddChan (item, CHAN_RAWVALUE, LXfECHAN_WRITE);
}

void Element::Eval (CLxUser_Evaluation &eval, CLxUser_Attributes &attr)
{
  /*
   *  The Eval function for the modifier reads input channels and writes
   *  output channels.
   */
  
  CLxUser_Value     chan_input;
  
  Value_Data    *value_obj = NULL;
  std::string     chan_output;
  unsigned     temp_chan_index = _chan_index;
  
  if (!eval || !attr)
    return;
  
  /*
   *  Read the input channel. We read this is a value object, and then get
   *  the pointer of the custom data using the Instrinsic function.
   */
  
  if (attr.ObjectRO (temp_chan_index++, chan_input) && chan_input.test ())
    value_obj = (Value_Data *) chan_input.Intrinsic ();
  
  /*
   *  Get the raw string value stored on the value object.
   */
  
  if (value_obj)
    chan_output = value_obj->GetString ();
  
  /*
   *  Write the value to the output channel.
   */
  
  attr.SetString (temp_chan_index++, chan_output.c_str ());
}

const char * Modifier::ItemType ()
{
  /*
   *  The modifier should only associate itself with this item type.
   */

  return ITEM_SERVER_NAME;
}

CLxItemModifierElement * Modifier::Alloc (CLxUser_Evaluation &eval, ILxUnknownID item)
{
  /*
   *  Allocate and return the modifier element.
   */

  return new Element (eval, item);
}

void initialize ()
{
  Instance::initialize ();
  Package::initialize ();
  Modifier::initialize ();
}

};  // End Namespace.