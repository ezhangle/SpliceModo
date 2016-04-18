#ifndef SRC_ITM_CANVASPI_H_
#define SRC_ITM_CANVASPI_H_

namespace CanvasPI
{
  struct piUserData;
  void initialize();
  class Instance;
  Instance *GetInstance(ILxUnknownID item_obj);
  piUserData *GetInstanceUserData(ILxUnknownID item_obj);
  BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
};

// constants.
#define MAX_NUM_VERTEX_FEATURE_OFFSETS  8

// default vertex map names.
#define VMAPNAME_UV   "CanvasUVs"

#endif  // SRC_ITM_CANVASPI_H_

