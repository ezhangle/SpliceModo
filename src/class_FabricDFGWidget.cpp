#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

#include "plugin.h"


FabricDFGWidget::FabricDFGWidget(QWidget *parent, BaseInterface *baseInterface) : DFG::DFGCombinedWidget(parent)
{
	if (baseInterface)
	{
		BaseInterface &b = *baseInterface;
		init( b.getClient(),
              b.getManager(),
			  b.getHost(),
			 *b.getBinding(),
			  b.getGraph(),
			  b.getStack(),
			  false);
	}
}

FabricDFGWidget::~FabricDFGWidget()
{
}

void FabricDFGWidget::onRecompilation()
{
  //... dirty node
}

void FabricDFGWidget::onPortRenamed(QString path, QString newName)
{
  // ... rename the dynamic attribute also in modo
}

