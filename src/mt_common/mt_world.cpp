#include "world.h"
#include "mayaSceneFactory.h"
#include "renderGlobalsFactory.h"
#include "utilities/logging.h"
#include <maya/MGlobal.h>
#include "../mt/swatchesRenderer/mtSwatchRenderer.h"
#include "../mt/SwatchesRenderer/SwatchesEvent.h"
#include <thread>

static Logging logger;
namespace MayaTo{

	void MayaToWorld::cleanUp()
	{
		MtSwatchRenderer * appleSwRndr = (MtSwatchRenderer *)this->getObjPtr("mtSwatchesRenderer");
		if (appleSwRndr)
			delete appleSwRndr;
	}

	void MayaToWorld::cleanUpAfterRender()
	{
		// after a normal rendering we do not need the maya scene data any more
		// remove it to save memory
		MayaSceneFactory().deleteMayaScene();
	}

	void MayaToWorld::initialize()
	{
		std::string oslShaderPath = (getRendererHome() + "shaders").asChar();
		MtSwatchRenderer *appleSwRndr = new MtSwatchRenderer();

		this->addObjectPtr("mtSwatchesRenderer", appleSwRndr);
		std::thread swatchRenderThread(MtSwatchRenderer::startMtSwatchRender, appleSwRndr);
		swatchRenderThread.detach();
	}

	void MayaToWorld::afterOpenScene()
	{
		Logging::debug("MayaToWorld::afterOpenScene");
	}

	void MayaToWorld::afterNewScene()
	{
		Logging::debug("MayaToWorld::afterNewScene");
	}

	void MayaToWorld::callAfterOpenCallback(void *)
	{
		getWorldPtr()->afterOpenScene();
	}
	void MayaToWorld::callAfterNewCallback(void *)
	{
		getWorldPtr()->afterNewScene();
	}

	void MayaToWorld::setRendererUnit()
	{
		this->rendererUnit = MDistance::kMeters;
	}

	void MayaToWorld::setRendererAxis()
	{
		this->rendererAxis = ZUp;
	}

}