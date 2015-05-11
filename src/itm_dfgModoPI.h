#ifndef SRC_ITM_DFGMODOPI_H_
#define SRC_ITM_DFGMODOPI_H_

namespace dfgModoPI
{
  void initialize();
  class Instance;
  Instance *GetInstance(ILxUnknownID item_obj);
  BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
};

#endif  // SRC_ITM_DFGMODOPI_H_

