#ifndef MT_H
#define MT_H

// Standard headers.
#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include <maya/MString.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MFnMeshData.h>

#include "rendering/renderer.h"
#include "../mt_common/mt_mayaObject.h"
#include "utilities/MiniMap.h"

class mtap_RenderGlobals;
class ShadingNode;

#define isBlack(x) ((x.r + x.g + x.b) <= 0.0f)

namespace MtRender
{
	class MtRenderer : public MayaTo::Renderer
	{
	public:
		MtRenderer();
		~MtRenderer();

		virtual void defineCamera(){};
		void defineCamera(std::shared_ptr<MayaObject> obj){};
		virtual void defineEnvironment(){};
		virtual void defineGeometry(){};
		void updateGeometry(std::shared_ptr<MayaObject> obj){};
		void updateInstance(std::shared_ptr<MayaObject> obj){};
		virtual void defineLights(){};
		void defineLight(std::shared_ptr<MayaObject> obj){};
		virtual void render(){};
		// initializeRenderer is called before rendering starts
		// it should prepare all data which can/should be reused during
		// ipr/frame/sequence rendering
		virtual void initializeRenderer(){};
		// unInitializeRenderer is called at the end of rendering.
		virtual void unInitializeRenderer(){};
		// updateShape is called during scene updating. If a renderer can update motionblur steps on the fly,
		// the geometry is defined at the very first step and later this definition will be updated for every motion step.
		// Other renderers will need a complete definition of all motionblur steps at once, so the motion steps will be
		// in the geometry e.g. with obj->addMeshData(); and at the very last step, everything is defined.
		virtual void updateShape(std::shared_ptr<MayaObject> obj){};
		// This method is necessary only if the renderer is able to update the transform definition interactively.
		// In other cases, the world space transform will be defined directly during the creation of the geometry.
		virtual void updateTransform(std::shared_ptr<MayaObject> obj){};
		virtual void abortRendering(){};
		virtual void interactiveFbCallback(){};
		virtual void doInteractiveUpdate(){};
		virtual void handleUserEvent(int event, MString strData, float floatData, int intData){};

	private:
		bool sceneBuilt = false;
	};

}

#endif