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

namespace Command { extern void initialize (); };
namespace Value   { extern void initialize (); };
namespace Item    { extern void initialize (); };

/*
 *  Initialize the servers.
 */

void initialize ()
{
  Command::initialize ();
  Value::initialize ();
  Item::initialize ();
}