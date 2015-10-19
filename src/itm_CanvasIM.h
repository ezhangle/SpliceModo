#ifndef SRC_ITM_CANVASIM_H_
#define SRC_ITM_CANVASIM_H_

namespace CanvasIM
{
  void initialize();
  class Instance;
  Instance *GetInstance(ILxUnknownID item_obj);
  BaseInterface *GetBaseInterface(ILxUnknownID item_obj);
};

#endif  // SRC_ITM_CANVASIM_H_
