#ifndef MT_SWATCH_RENDERER
#define MT_SWATCH_RENDERER

#include "swatchesRenderer/SwatchRendererInterface.h"

class mt_SwatchRendererInterface : SwatchRendererInterface
{
public:
	mt_SwatchRendererInterface(MObject dependNode, MObject renderNode, int imageResolution);
	~mt_SwatchRendererInterface();
	MObject renderNode;
	MObject dependNode;

	virtual void init();
	virtual void loadGeometry();
	virtual void renderSwatch();
	virtual void getImage(MImage& imageRef);
	virtual void getImageData(MImage& imageRef);

	virtual void fillDummySwatch(MImage& image);
};
#endif