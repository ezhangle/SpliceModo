#ifndef SRC_ITM_COMMON_H_
#define SRC_ITM_COMMON_H_

namespace ItemCommon
{
  LxResult pins_Newborn(ILxUnknownID original, unsigned flags, ILxUnknownID item_obj, BaseInterface *baseInterface);
  LxResult pins_AfterLoad(ILxUnknownID item_obj, BaseInterface *baseInterface);
  void pins_Doomed(BaseInterface *baseInterface);
  LxResult pkg_SetupChannels(ILxUnknownID addChan_obj, bool addObjRefChannel);
  LxResult cui_UIHints(const char *channelName, ILxUnknownID hints_obj);
  bool Test(ILxUnknownID item_obj, std::vector <ModoTools::UsrChnDef> &m_usrChan);
};

#endif  // SRC_ITM_COMMON_H_
