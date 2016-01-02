#include "swatchesRenderer/swatchRendererInterfaceFactory.h"
#include "mt_swatchRenderer.h"


SwatchRendererInterface *SwatchRendererInterfaceFactory::createSwatchRendererInterface(MObject dependNode, MObject renderNode, int imageResolution)
{
	return (SwatchRendererInterface *)new mt_SwatchRendererInterface(dependNode, renderNode, imageResolution);
}

void SwatchRendererInterfaceFactory::deleteSwatchRendererInterface(SwatchRendererInterface *swInterface)
{
	mt_SwatchRendererInterface *sr = (mt_SwatchRendererInterface *)swInterface;
	if (sr != NULL)
		delete sr;
}