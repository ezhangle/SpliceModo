#ifndef SRC__CLASS_JSONVALUE_H_
#define SRC__CLASS_JSONVALUE_H_

#include "lxidef.h"

#include <lx_action.hpp>
#include "lx_chanmod.hpp"
#include <lx_channelui.hpp>
#include <lx_draw.hpp>
#include "lx_item.hpp"
#include "lx_io.hpp"
#include "lx_package.hpp"
#include "lx_plugin.hpp"
#include "lx_value.hpp"
#include <lx_wrap.hpp>

#include <lxu_attrdesc.hpp>
#include "lxu_log.hpp"
#include <lxu_math.hpp>
#include "lxu_modifier.hpp"
#include "lxu_select.hpp"

#include "lxlog.h"
#include <lxvmath.h>
#include "lx_wrap.hpp"

class BaseInterface;

// The Value class implements the custom value type. The base interface for this
// object is the Value Interface. This provides the basic functions for
// manipulating the value. We also implement a StreamIO interface, allowing us
// to read and write the custom value to the scene file.

class JSONValue : public CLxImpl_Value,
                  public CLxImpl_StreamIO
{
  public:
  static void initialize ()
  {
    CLxGenericPolymorph *srv = NULL;

    srv = new CLxPolymorph                 <JSONValue>;
    srv->AddInterface(new CLxIfc_Value     <JSONValue>);
    srv->AddInterface(new CLxIfc_StreamIO  <JSONValue>);
    srv->AddInterface(new CLxIfc_StaticDesc<JSONValue>);

    lx::AddServer(SERVER_NAME_dfgModoIM ".jsonvalue", srv);
  }
  
  struct _JSONValue
  {
    std::string   s;
    BaseInterface *baseInterface;
    void zero()
    {
      s.clear();
      baseInterface = NULL;
    }
  };

  static LXtTagInfoDesc descInfo[];
  _JSONValue m_data;

  JSONValue()   { m_data.zero(); }
  ~JSONValue()  {                }
  
  unsigned int val_Type()                               LXx_OVERRIDE { return LXi_TYPE_OBJECT; }
  LxResult     val_Copy(ILxUnknownID other)             LXx_OVERRIDE;
  LxResult     val_GetString(char *buf, unsigned len)   LXx_OVERRIDE;
  LxResult     val_SetString(const char *val)           LXx_OVERRIDE;
  void        *val_Intrinsic()                          LXx_OVERRIDE;
  
  LxResult   io_Write(ILxUnknownID stream)  LXx_OVERRIDE;
  LxResult   io_Read (ILxUnknownID stream)  LXx_OVERRIDE;
};

#endif  // SRC__CLASS_JSONVALUE_H_

