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

namespace Value
{

/*
 *  The Value class implements the custom value type. The base interface for this
 *  object is the Value Interface. This provides the basic functions for
 *  manipulating the value. We also implement a StreamIO interface, allowing us
 *  to read and write the custom value to the scene file.
 */

class Value : public CLxImpl_Value,
              public CLxImpl_StreamIO
{
  public:
    static void initialize ()
    {
      CLxGenericPolymorph  *srv = NULL;

      srv = new CLxPolymorph            <Value>;
      srv->AddInterface    (new CLxIfc_Value    <Value>);
      srv->AddInterface    (new CLxIfc_StreamIO    <Value>);
      srv->AddInterface    (new CLxIfc_StaticDesc    <Value>);

      lx::AddServer      (VALUE_SERVER_NAME, srv);
    }
  
    Value ()
    {
      _data = new Value_Data;
    }
  
    ~Value ()
    {
      if (_data)
        delete _data;
    }
  
    unsigned int   val_Type    ()        LXx_OVERRIDE;
    LxResult   val_Copy    (ILxUnknownID other)    LXx_OVERRIDE;
    LxResult   val_SetInt    (int val)      LXx_OVERRIDE;
    LxResult   val_GetString    (char *buf, unsigned len)  LXx_OVERRIDE;
    LxResult   val_SetString    (const char *val)    LXx_OVERRIDE;
    void    *val_Intrinsic    ()        LXx_OVERRIDE;
  
    LxResult   io_Write    (ILxUnknownID stream)    LXx_OVERRIDE;
    LxResult   io_Read    (ILxUnknownID stream)    LXx_OVERRIDE;
  
    static LXtTagInfoDesc   descInfo[];
  
  private:
    Value_Data    *_data;
};

unsigned int Value::val_Type ()
{
  /*
   *  Our custom value is an object. So this function is pretty self
   *  explanatory. For the majority of custom values, this function should
   *  always return object.
   */

  return LXi_TYPE_OBJECT;
}

LxResult Value::val_Copy (ILxUnknownID other)
{
  /*
   *  Copy another instance of our custom value to this one. We just cast
   *  the object to our internal structure and then copy the data.
   */

  Value_Data    *data = NULL;
  
  if (other && _data)
  {
    data = static_cast <Value_Data *> ((void *) other);
    
    if (data)
    {
      _data->SetString (data->GetString ());
      
      return LXe_OK;
    }
  }
  
  return LXe_FAILED;
}
  
LxResult Value::val_SetInt (int val)
{
  /*
   *  Our custom value type has functions for setting the value using an
   *  integer, so we may as well implement this function - it doesn't really
   *  matter though, the custom value object is pretty arbitrary.
   */
  
  if (_data)
  {
    _data->SetInt (val);
    
    return LXe_OK;
  }
  
  return LXe_FAILED;
}

LxResult Value::val_GetString (char *buf, unsigned len)
{
  /*
   *  This function - as the name suggests - is used to get the custom value
   *  as a string. We just read the string from the custom value object. As
   *  the caller provides a buffer and length, we should test the length of
   *  the buffer against our string length, and if it's too short, return
   *  short buffer. The caller will then provide a bigger buffer for us to
   *  copy the string into.
   */
  
  std::string     string;
  
  if (buf && _data)
  {
    string = _data->GetString ();
    
    if (string.size () >= len)
      return LXe_SHORTBUFFER;
    
    strncpy (buf, string.c_str (), len);
    
    return LXe_OK;
  }
  
  return LXe_FAILED;
}

LxResult Value::val_SetString (const char *val)
{
  /*
   *  Similar to the get string function, this function sets the string.
   *  Again, this is completely arbitrary, but we may as well provide the
   *  functionality as an example.
   */
  
  if (val && _data)
  {
    _data->SetString (std::string (val));
    
    return LXe_OK;
  }
  
  return LXe_FAILED;
}
  
void * Value::val_Intrinsic ()
{
  /*
   *  The Intrinsic function is the important one. This returns a pointer
   *  to our custom class, allowing callers to interface with it directly.
    */

  return _data;
}

LxResult Value::io_Write (ILxUnknownID stream)
{
  /*
   *  The Write function is called whenever the custom value type is being
   *  written to a stream, for example, writing to a scene file. We simply
   *  write the raw string value into the stream.
   */
  
  CLxUser_BlockWrite   write (stream);
  std::string     string;
  
  if (_data && write.test ())
  {
    string = _data->GetString ();
  
    return write.WriteString (string.c_str ());
  }
  
  return LXe_FAILED;
}

LxResult Value::io_Read (ILxUnknownID stream)
{
  /*
   *  The Read function is called whenever the custom value type is being
   *  read from a stream, for example, loading from a scene file. We simply
   *  read the raw string value from the stream, and use it to setup our
   *  custom value data.
   */
  
  CLxUser_BlockRead   read (stream);
  std::string     string;
  
  if (_data && read.test ())
  {
    if (read.Read (string))
    {
      _data->SetString (string);
      
      return LXe_OK;
    }
  }
  
  return LXe_FAILED;
}

LXtTagInfoDesc Value::descInfo[] =
{
  { 0 }
};

void initialize ()
{
  Value::initialize ();
}

};  // End Namespace.