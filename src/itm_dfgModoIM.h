#ifndef SRC_ITM_DFGMODOIM_H_
#define SRC_ITM_DFGMODOIM_H_

namespace dfgModoIM
{
    void initialize();
    class Instance;
    Instance *GetInstance(ILxUnknownID item_obj);
    BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
    void InvalidateItem(void *ILxUnknownID_item_obj);
};

#endif  // SRC_ITM_DFGMODOIM_H_
