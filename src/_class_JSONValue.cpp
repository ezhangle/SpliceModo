#include "plugin.h"

#include "_class_BaseInterface.h"
#include "_class_JSONValue.h"

#define JSONVALUE_DEBUG_LOG   0   // if != 0 then log info when reading/writing JSONValue channels.

LxResult JSONValue::val_Copy(ILxUnknownID other)
{
  /*
    Copy another instance of our custom value to this one. We just cast
    the object to our internal structure and then copy the data.
  */

  //char t[256];
  //sprintf(t, "JSONValue::val_Copy, m_data = %ld, other = %ld, *other = %ld", (int)m_data, (int)other, other ? (int)(*other) : 0);
  //feLog(t);

  _JSONValue *other_data = NULL;
  if (other && m_data)
  {
    other_data = static_cast <_JSONValue *>((void *)other);
    if (other_data)
    {
      if (   other_data->chnIndex >= -1
          && other_data->chnIndex <  CHN_FabricJSON_NUM)  // QUICK HACK to prevent crash from FE-6090!
      {
        m_data->chnIndex      = other_data->chnIndex;
        m_data->s             = other_data->s;
        m_data->baseInterface = other_data->baseInterface;
      }
      return LXe_OK;
    }
  }

  return LXe_FAILED;
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
  
  if (!m_data || !buf)
    return LXe_FAILED;

  if (m_data->s.size() >= len)
    return LXe_SHORTBUFFER;

  if (!m_data->s.empty())
    strcpy(buf, m_data->s.c_str());

  return LXe_OK;
}

LxResult JSONValue::val_SetString(const char *val)
{
  /*
    Similar to the get string function, this function sets the string.
  */

  if (!m_data || !val)
    return LXe_FAILED;

  m_data->s = val;

  return LXe_OK;
}

void *JSONValue::val_Intrinsic()
{
  /*
    The Intrinsic function is the important one. This returns a pointer
    to the custom value's class, allowing callers to interface with it directly.
  */
  return m_data;
}

LxResult JSONValue::io_Write(ILxUnknownID stream)
{
  /*
    The Write function is called whenever the custom value type is being
    written to a stream, for example, writing to a scene file. 

    NOTE: we do not write the string m_data.s, instead we write
          the JSON string BaseInterface::getJSON().

    [FE-4927] unfortunately these types of channels can only store 2^16 bytes,
              so until that limitation is present the JSON string is split
              into chunks of size CHN_FabricJSON_MAX_BYTES and divided over
              all the CHN_NAME_IO_FabricJSON channels.
              Not the prettiest workaround, but it works.
  */
  CLxUser_BlockWrite write(stream);
  char preLog[128];
  sprintf(preLog, "JSONValue::io_Write(m_data.chnIndex = %ld)", m_data->chnIndex);

  if (!write.test())          return LXe_FAILED;
  if (!m_data)                return LXe_FAILED;

  // note: we never write 'nothing' (zero bytes) or else
  // the CHN_NAME_IO_FabricJSON channels won't get properly
  // initialized when loading a scene.
  char pseudoNothing[8] = " ";  // one byte of data.

  // write the JSON string.
  if (m_data->chnIndex < 0)
  { feLog(std::string(preLog) + ": chnIndex is less than 0.");
    return write.WriteString(pseudoNothing); }
  if (!m_data->baseInterface)
  { feLog(std::string(preLog) + ": pointer at BaseInterface is NULL.");
    return write.WriteString(pseudoNothing); }
  try
  {
    // get the JSON string and its length.
    std::string json = m_data->baseInterface->getJSON();
    uint32_t len = json.length();

    // trivial case, i.e. nothing to write?
    if ((uint32_t)m_data->chnIndex * CHN_FabricJSON_MAX_BYTES >= len)
      return write.WriteString(pseudoNothing);

    // string too long?
    if (len > (uint32_t)CHN_FabricJSON_NUM * CHN_FabricJSON_MAX_BYTES)
    {
      if (m_data->chnIndex == 0)
      {
        char log[256];
        sprintf(log, ": the JSON string is %ld long!", len);
        feLogError(std::string(preLog) + log);
        sprintf(log, ": it exceeds the max size of %ld bytes!", (uint32_t)CHN_FabricJSON_NUM * CHN_FabricJSON_MAX_BYTES);
        feLogError(std::string(preLog) + log);
      }
      return LXe_FAILED;
    }

    // extract the part that will be saved for this channel.
    std::string part;
    part = json.substr((uint32_t)m_data->chnIndex * CHN_FabricJSON_MAX_BYTES, CHN_FabricJSON_MAX_BYTES);
    if (part.length() == 1)
      part += " ";

    // write.
    if (JSONVALUE_DEBUG_LOG)
    {
      char log[128];
      sprintf(log, ": writing %.1f kilobytes (%ld bytes)", (float)part.length() / 1024.0, (long)part.length());
      feLog(std::string(preLog) + log);
    }
    if (part.length() && part.c_str())   return write.WriteString(part.c_str());
    else                                 return write.WriteString(pseudoNothing);
  }
  catch (FabricCore::Exception e)
  {
    std::string err = std::string(preLog) + ": ";
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

  if (!m_data)
    return LXe_FAILED;

  CLxUser_BlockRead read(stream);
  char preLog[128];
  sprintf(preLog, "JSONValue::io_Read()");

  if (read.test() && read.Read(m_data->s))
  {
    if (JSONVALUE_DEBUG_LOG && m_data->s.length() > 1)
    {
      char log[128];
      sprintf(log, ": read %.1f kilobytes (%ld bytes)", (float)m_data->s.length() / 1024.0, (long)m_data->s.length());
      feLog(std::string(preLog) + log);
    }
    return LXe_OK;
  }
  else
  {
    if (JSONVALUE_DEBUG_LOG)
      feLog(std::string(preLog) + ": read error");
    m_data->s = " ";
    return LXe_FAILED;
  }
}

LXtTagInfoDesc JSONValue::descInfo[] =
{
  { LXsSRV_LOGSUBSYSTEM, LOG_SYSTEM_NAME },
  { 0 }
};
