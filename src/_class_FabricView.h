#ifndef SRC__CLASS_FABRICVIEW_H_
#define SRC__CLASS_FABRICVIEW_H_

#include "lxw_customview.hpp"

class FabricDFGWidget;
void feLog(const std::string &s);

class FabricView : public CLxImpl_CustomView
{
 public:

  LxResult customview_Init(ILxUnknownID pane);

  // to be called within the plugin initialize
  static void initialize();

  //
  FabricView()
  {
    // add pointer to static std::vector.
    s_FabricViews.push_back(this);
  }
  ~FabricView()
  {
    // remove pointer from static std::vector.
    for (size_t i = 0; i < s_FabricViews.size(); i++)
      if (s_FabricViews[i] == this)
      {
        s_FabricViews.erase(s_FabricViews.begin() + i);
        break;
      }
  }

  // to be used when constructing the FabricDFGWidget
  QWidget * parentWidget();

  // setter / getter for the contained FabricDFGWidget
  FabricDFGWidget * widget();
  void setWidget(FabricDFGWidget * dfgWidget);
  void setWidgetNULL();

  // static vector of pointers.
  static std::vector <FabricView *> s_FabricViews;

 private:

  QWidget * m_parentWidget;
  FabricDFGWidget * m_dfgWidget;
};

#endif  // SRC__CLASS_FABRICVIEW_H_

