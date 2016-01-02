#include "mt_swatchRenderer.h"

#include "utilities/tools.h"
#include "utilities/attrTools.h"
#include "utilities/logging.h"

mt_SwatchRendererInterface::mt_SwatchRendererInterface(MObject dependNode, MObject renderNode, int imageResolution)
{
#ifdef _DEBUG
	Logging::setLogLevel(Logging::Debug);
#endif

	this->imgDone = false;
	this->inProgress = false;

	this->dependNode = dependNode;
	this->renderNode = renderNode;
	this->swatchWidth = imageResolution;
	this->imageData = (float *)malloc(sizeof(float)* this->swatchWidth * this->swatchWidth * 4);
	memset(this->imageData, 0, sizeof(float)* this->swatchWidth * this->swatchWidth * 4);
}

mt_SwatchRendererInterface::~mt_SwatchRendererInterface()
{
	if (this->imageData != nullptr)
		free(this->imageData);
	this->imageData = nullptr;
	Logging::debug("SwatchRendererInterface deleted.");
}

void mt_SwatchRendererInterface::init()
{
	Logging::debug("SwatchRendererInterface init called.");
}

void mt_SwatchRendererInterface::loadGeometry(){}

void mt_SwatchRendererInterface::fillDummySwatch(MImage& image)
{
	const int res(swatchWidth);
	float rndR = rnd();
	float rndG = rnd();
	float rndB = rnd();

	float *pixels = image.floatPixels();
	int index = 0;
	for (int y = 0; y < res; y++)
	{
		for (int x = 0; x < res; x++)
		{
			float fac = float(y) / res;
			pixels[index++] = fac * rndR;
			pixels[index++] = fac * rndG;
			pixels[index++] = fac * rndB;
			pixels[index++] = 1.0f;
		}
	}
}


void mt_SwatchRendererInterface::getImageData(MImage& imageRef)
{
	fillDummySwatch(imageRef);

	//float *img = imageRef.floatPixels();
	//memcpy(img, this->imageData, sizeof(float) * this->swatchWidth * this->swatchWidth * 4);
	return;
}


void mt_SwatchRendererInterface::renderSwatch()
{

}

void mt_SwatchRendererInterface::getImage(MImage& imageRef)
{}

//void mt_SwatchRendererInterface::swatchRenderThread(mt_SwatchRendererInterface* me)
//{
//
//}
//
//void mt_SwatchRendererInterface::initializeStaticData()
//{
//#ifdef _DEBUG
//	Logging::setLogLevel(Logging::Debug);
//#endif
//}
//
//void mt_SwatchRendererInterface::cleanUpStaticData()
//{
//}