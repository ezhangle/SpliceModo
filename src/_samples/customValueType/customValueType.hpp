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

#ifndef CUSTOMVALUETYPE_HPP
#define  CUSTOMVALUETYPE_HPP

/*
 *  Include the required source files.
 */

#include <lxidef.h>

#include <lx_io.hpp>
#include <lx_package.hpp>
#include <lx_plugin.hpp>
#include <lx_value.hpp>
#include <lx_wrap.hpp>

#include <lxu_command.hpp>
#include <lxu_modifier.hpp>
#include <lxu_select.hpp>

#include <sstream>

/*
 *  The value name is the name of the custom value we're implementing; for example
 *  integer, float, time, string...etc. The first character in the name is a +
 *  sign, this specifies that the value type has been implemented externally.
 */

#define ITEM_SERVER_NAME    "customValue.item"
#define COMMAND_SERVER_NAME "customValue.set"
#define VALUE_SERVER_NAME   "customValue"

#define VALUE_TYPE_NAME     "+"VALUE_SERVER_NAME

#define CHAN_CUSTOMVALUE    "customValue"
#define CHAN_RAWVALUE       "rawValue"
#define ARG_VALUE           "value"

static CLxItemType   gItemType (ITEM_SERVER_NAME);

/*
 *  This is our value data. When our value type is evaluated, the object returned
 *  will point to this class, allowing us to call functions on it to get and set
 *  the data. It's pretty useless, and the functions are completely arbitrary, but
 *  it demonstrates the basics.
 */

class Value_Data
{
 public:
  Value_Data() : m_value ("Uninitialized") { }
  
  Value_Data(std::string &value)
  {
    m_value = value;
  }
  
  void SetInt(int value)
  {
    std::stringstream ss;
    ss << value;
    m_value = ss.str();
  }
  
  void SetString(std::string value)
  {
    m_value = value;
  }
  
  std::string GetString()
  {
    return m_value;
  }
  
 private:
  std::string m_value;
};

#endif  // CUSTOMVALUETYPE_HPP