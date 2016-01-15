#ifndef SRC_ITM_COMMON_H_
#define SRC_ITM_COMMON_H_

namespace ItemCommon
{
  void pins_Doomed(BaseInterface *baseInterface);
  LxResult pkg_SetupChannels(ILxUnknownID addChan_obj, bool addObjRefChannel);
  LxResult cui_UIHints(const char *channelName, ILxUnknownID hints_obj);
};

#endif  // SRC_ITM_COMMON_H_
