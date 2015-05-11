#ifndef SRC_ITM_DFGMODOPIOLD_H_
#define SRC_ITM_DFGMODOPIOLD_H_

namespace dfgModoPIold
{
    void initialize();
    class CReadItemInstance;
    CReadItemInstance *GetInstance(ILxUnknownID item_obj);
    BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
};

#define CHN_NAME_IO_time    "time"
#define CHN_NAME_IO_frame   "frame"
#define CHN_NAME_IO_matrix  "matrix"

#endif  // SRC_ITM_DFGMODOPIOLD_H_

