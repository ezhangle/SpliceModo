#ifndef SRC_ITM_CANVASPIPILOT_H_
#define SRC_ITM_CANVASPIPILOT_H_

namespace CanvasPIpilot
{
    void initialize();
    class CReadItemInstance;
    CReadItemInstance *GetInstance(ILxUnknownID item_obj);
    BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
};

#define CHN_NAME_IO_time    "time"
#define CHN_NAME_IO_frame   "frame"
#define CHN_NAME_IO_matrix  "matrix"
#define CHN_NAME_IO_FabricDisplay "FabricDisplay"
#define CHN_NAME_IO_FabricOpacity "FabricOpacity"

#endif  // SRC_ITM_CANVASPIPILOT_H_

