#ifndef SRC_ITM_DFGMODOPIOLD_H_
#define SRC_ITM_DFGMODOPIOLD_H_

namespace dfgModoPIold
{
    void initialize();
    class CReadItemInstance;
    CReadItemInstance *GetInstance(ILxUnknownID item_obj);
    BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
};

#endif  // SRC_ITM_DFGMODOPIOLD_H_

