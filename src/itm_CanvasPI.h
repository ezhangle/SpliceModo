#ifndef SRC_ITM_CANVASPI_H_
#define SRC_ITM_CANVASPI_H_

namespace CanvasPI
{
  void initialize();
  class Instance;
  Instance *GetInstance(ILxUnknownID item_obj);
  BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
};

#endif  // SRC_ITM_CANVASPI_H_

