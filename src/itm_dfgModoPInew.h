#ifndef SRC_ITM_DFGMODOPINEW_H_
#define SRC_ITM_DFGMODOPINEW_H_

namespace dfgModoPInew
{
  void initialize();
  class Instance;
  Instance *GetInstance(ILxUnknownID item_obj);
  BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
};

#endif  // SRC_ITM_DFGMODOPINEW_H_

