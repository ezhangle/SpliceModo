#ifndef _FABRICVIEW_H_
#define _FABRICVIEW_H_

#include "_class_ModoTools.h"
#include "_class_FabricDFGWidget.h"

class FabricView : public CLxImpl_CustomView
{
public:

  LxResult customview_Init(ILxUnknownID pane);

  // to be called within the plugin initialize
  static void initialize();

  // to be used when constructing the FabricDFGWidget
  QWidget * parentWidget();

  // setter / getter for the contained FabricDFGWidget
  FabricDFGWidget * widget();
  void setWidget(FabricDFGWidget * dfgWidget);

private:

  QWidget * m_parentWidget;
  FabricDFGWidget * m_dfgWidget;
};

#endif _FABRICVIEW_H_
