#include "MtSwatchRenderer.h"

#include "SwatchesEvent.h"
#include "swatchesRenderer/swatchesQueue.h"
#include "swatchesRenderer/newSwatchRenderer.h"
#include "swatchesRenderer/swatchGeometry.h"
#include <maya/MFnDependencyNode.h>
#include "utilities/logging.h"
#include "utilities/tools.h"
#include <thread>
#include "shadingTools/material.h"
#include "shadingTools/shadingUtils.h"
#include "world.h"
#include <maya/MGlobal.h>

MtSwatchRenderer::MtSwatchRenderer()
{
	Logging::debug(MString("Initialze swatch renderer."));
}

void MtSwatchRenderer::renderSwatch()
{}

void MtSwatchRenderer::renderSwatch(NewSwatchRenderer *sr)
{
	int res(sr->resolution());
	this->setSize(res);
	this->setShader(sr->dNode);

	sr->image().create(res, res, 4, MImage::kFloat);
	float *floatPixels = sr->image().floatPixels();
	this->fillSwatch(floatPixels);
}

void MtSwatchRenderer::fillSwatch(float *pixels)
{
}

void MtSwatchRenderer::setSize(int size)
{
	MString res = MString("") + size + " " + size;
}

void MtSwatchRenderer::setShader(MObject shader)
{
	this->defineMaterial(shader);
}

MtSwatchRenderer::~MtSwatchRenderer()
{
	terminateMtSwatchRender(this);
	Logging::debug("Removing MtSwatchRenderer.");
}

void MtSwatchRenderer::mainLoop()
{
#ifdef _DEBUG
	Logging::setLogLevel(Logging::Debug);
#endif

	SQueue::SEvent swatchEvent;
	Logging::debug("Starting MtSwatchRenderer main Loop.");
	while (!terminateLoop)
	{
		SQueue::getQueue()->wait_and_pop(swatchEvent);
		Logging::debug(MString("MtSwatchRenderer main Loop: renderDoneAddr: ") + (int)swatchEvent.renderDone);

		if (swatchEvent.renderDone == nullptr)
		{
			Logging::debug(MString("MtSwatchRenderer main Loop: received a null ptr. Terminating loop"));
			terminateLoop = true;
		}
		else{
			//this->setShader(swatchEvent.shadingNode);
			//this->setSize(swatchEvent.height);
			//std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			//this->renderSwatch();
			//this->fillSwatch(swatchEvent.pixels);
			swatchEvent.swatchRenderer->finishParallelRender();
			*swatchEvent.renderDone = true;
		}
	}
	loopDone = true;
}


void MtSwatchRenderer::defineMaterial(MObject shadingNode)
{
	MStatus status;
	// to use the unified material function we need the shading group
	// this works only for surface shaders, textures can be handled differently later
	MPlugArray pa, paOut;
	MFnDependencyNode depFn(shadingNode);
	depFn.getConnections(pa);
	for (uint i = 0; i < pa.length(); i++)
	{
		if (pa[i].isDestination())
			continue;
		pa[i].connectedTo(paOut, false, true);
		if (paOut.length() > 0)
		{
			for (uint k = 0; k < paOut.length(); k++)
			{
				if (paOut[k].node().hasFn(MFn::kShadingEngine))
				{
					MObject outNode = paOut[k].node();
					//AppleRender::updateMaterial(outNode, assembly);
					break;
				}
			}
		}
	}
}

void MtSwatchRenderer::startMtSwatchRender(MtSwatchRenderer *swRend)
{
	if (swRend != nullptr)
	{
		Logging::debug(MString("startMtSwatchRender"));
		swRend->mainLoop();
		delete swRend;
		Logging::debug(MString("startMtSwatchRender done and deleted."));
	}
}

void MtSwatchRenderer::terminateMtSwatchRender(MtSwatchRenderer *swRend)
{
	Logging::debug(MString("terminateMtSwatchRender"));
	SQueue::SEvent swatchEvent;
	SQueue::SwatchesQueue.push(swatchEvent);
	swRend->terminateLoop = true;
	while (!swRend->loopDone)
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
}
