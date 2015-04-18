#include <stdio.h>
#include "plugin.h"

std::vector <FabricView *> FabricView::s_FabricViews;

LxResult FabricView::customview_Init(ILxUnknownID pane)
{
  CLxLoc_CustomPane p(pane);

  if (!p.test())
    return LXe_FAILED;

  void* parent = NULL;
  p.GetParent(&parent);
  if (!parent)
    return LXe_FAILED;

  m_parentWidget = static_cast<QWidget*>(parent);
  m_parentWidget->setContentsMargins(0, 0, 0, 0);
  QVBoxLayout * layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  m_parentWidget->setLayout(layout);

  m_dfgWidget = NULL;

  return LXe_OK;
}

QWidget *FabricView::parentWidget()
{
  return m_parentWidget;
}

FabricDFGWidget *FabricView::widget()
{
  return m_dfgWidget;
}

void FabricView::setWidget(FabricDFGWidget *dfgWidget)
{
  if (m_dfgWidget)
  {
    m_dfgWidget->close();
    m_dfgWidget->deleteLater();
  }
  m_dfgWidget = dfgWidget;

  // add the new widget to the parent widget's layout
  QVBoxLayout *layout = (QVBoxLayout *)m_parentWidget->layout();
  layout->addWidget(m_dfgWidget);
}

void FabricView::initialize()
{
  CLxGenericPolymorph* cmd = NULL;
  cmd = new CLxPolymorph<FabricView>();
  cmd->AddInterface(new CLxIfc_CustomView< FabricView >() );
  lx::AddServer("FabricCanvas", cmd);
}
