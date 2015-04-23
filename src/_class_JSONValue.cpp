#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_JSONValue.h"


LxResult JSONValue::val_Copy(ILxUnknownID other)
{
  /*
    Copy another instance of our custom value to this one. We just cast
    the object to our internal structure and then copy the data.
  */

  if (!other)     return LXe_FAILED;

  _JSONValue *otherData = (_JSONValue *)((void *)other);
  if (!otherData) return LXe_FAILED;
  
  m_data.s             = otherData->s;
  m_data.baseInterface = NULL;

  return LXe_OK;
}

LxResult JSONValue::val_GetString(char *buf, unsigned len)
{
  /*
    This function - as the name suggests - is used to get the custom value
    as a string. We just read the string from the custom value object. As
    the caller provides a buffer and length, we should test the length of
    the buffer against our string length, and if it's too short, return
    short buffer. The caller will then provide a bigger buffer for us to
    copy the string into.
  */
  
  if (!buf)       return LXe_FAILED;

  if (m_data.s.size() >= len)
    return LXe_SHORTBUFFER;

  strncpy(buf, m_data.s.c_str(), len);

  return LXe_OK;
}

LxResult JSONValue::val_SetString(const char *val)
{
  /*
    Similar to the get string function, this function sets the string.
  */

  if (!val)       return LXe_FAILED;

  m_data.s = val;
    
  return LXe_OK;
}

void *JSONValue::val_Intrinsic()
{
  /*
    The Intrinsic function is the important one. This returns a pointer
    to the custom value's class, allowing callers to interface with it directly.
  */
  return (void *)&m_data;
}

LxResult JSONValue::io_Write(ILxUnknownID stream)
{
  /*
    The Write function is called whenever the custom value type is being
    written to a stream, for example, writing to a scene file. 

    NOTE: we do not write the string m_data.s, instead we write
          the JSON string BaseInterface::getJSON().
  */
  CLxUser_BlockWrite write(stream);
  feLog("JSONValue::io_Write()");

  if (!write.test())  return LXe_FAILED;

  // write the JSON string.
  if (!m_data.baseInterface)
  { feLogError("JSONValue::io_Write(): pointer at BaseInterface is NULL!");
    return LXe_FAILED;  }
  try
  {
    std::string json = m_data.baseInterface->getJSON();
    return write.WriteString(json.c_str());
  }
  catch (FabricCore::Exception e)
  {
    std::string err = "JSONValue::io_Write(): ";
    err += (e.getDesc_cstr() ? e.getDesc_cstr() : "\"\"");
    feLogError(err);
    return LXe_FAILED;
  }
}

LxResult JSONValue::io_Read(ILxUnknownID stream)
{
  /*
    The Read function is called whenever the custom value type is being
    read from a stream, for example, loading from a scene file. 

    NOTE: the string is read into m_data.s and will then be used in
          the function Instance::pins_AfterLoad() to set the graph
          via BaseInterface::setFromJSON().
  */

  CLxUser_BlockRead read(stream);
  feLog("JSONValue::io_Read()");

  if (!read.test())  return LXe_FAILED;

  if (read.Read(m_data.s))
    return LXe_OK;
  else
    return LXe_FAILED;
}

LXtTagInfoDesc JSONValue::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};
