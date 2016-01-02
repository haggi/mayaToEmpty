#include <maya/MPlugArray.h>
#include <maya/MPlug.h>
#include "mt_mayaRenderer.h"
#include "utilities/logging.h"
#include "utilities/attrTools.h"
#include "shadingTools/material.h"
#include "shadingTools/shadingUtils.h"
#include "world.h"

#include <thread>
#include <vector>
#include <maya/MGlobal.h>
#include <maya/MStringArray.h>
#include <maya/MFnDependencyNode.h>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#if MAYA_API_VERSION >= 201600

#define kNumChannels 4

mt_MayaRenderer::mt_MayaRenderer()
{
	initProject();
	width = height = initialSize;
	this->rb = (float*)malloc(width*height*kNumChannels*sizeof(float));
}

mt_MayaRenderer::~mt_MayaRenderer()
{
	Logging::debug("~mt_MayaRenderer");
	objectArray.clear();
}

void mt_MayaRenderer::initEnv()
{
	MString mayaRoot = getenv("MAYA_LOCATION");
	MString envMapAttrName = mayaRoot + "/presets/Assets/IBL/Exterior1_Color.exr";
}

void mt_MayaRenderer::initProject()
{}

bool mt_MayaRenderer::isRunningAsync()
{
	Logging::debug("isRunningAsync");
	return true;
}
void* mt_MayaRenderer::creator()
{
	return new mt_MayaRenderer();
}

void mt_MayaRenderer::render()
{
	ProgressParams pp;
	pp.progress = 0.0;
	progress(pp);

	Logging::debug("renderthread done.");

	pp.progress = 2.0;
	progress(pp);

}

static void startRenderThread(mt_MayaRenderer* renderPtr)
{
	renderPtr->render();
}

MStatus mt_MayaRenderer::startAsync(const JobParams& params)
{
	Logging::debug("startAsync:");
	Logging::debug(MString("\tJobDescr: ") + params.description + " max threads: " + params.maxThreads);
	Logging::debug(MString("started async"));
	asyncStarted = true;
	return MStatus::kSuccess;
};
MStatus mt_MayaRenderer::stopAsync()
{
	Logging::debug("stopAsync");
	if (renderThread.joinable())
		renderThread.join();
	asyncStarted = false;
	return MS::kSuccess;
}

MStatus mt_MayaRenderer::beginSceneUpdate()
{
	Logging::debug("beginSceneUpdate");
	if (renderThread.joinable())
		renderThread.join();
	return MStatus::kSuccess;
};

MStatus mt_MayaRenderer::translateMesh(const MUuid& id, const MObject& node)
{
	MObject mobject = node;
	MFnDependencyNode depFn(mobject);
	MString meshName = depFn.name();
	MString meshIdName = meshName;
	MString meshInstName = meshIdName + "_instance";
	Logging::debug(MString("translateMesh ") + meshIdName);

	IdNameStruct idName;
	idName.id = id;
	idName.name = meshIdName;
	idName.mobject = node;
	objectArray.push_back(idName);
	lastShapeId = id;
	return MStatus::kSuccess;
};

MStatus mt_MayaRenderer::translateLightSource(const MUuid& id, const MObject& node)
{
	MFnDependencyNode depFn(node);
	MString lightName = depFn.name();
	MString lightInstName = lightName + "_instance";
	MString lightIdName = lightName + "_" + id;

	Logging::debug(MString("translateLightSource: ") + depFn.name() + " from type: " + node.apiTypeStr());
	if (node.hasFn(MFn::kAreaLight))
	{		
		IdNameStruct idName;
		idName.id = id;
		idName.name = lightIdName;
		idName.mobject = node;
		objectArray.push_back(idName);
		lastShapeId = id;
	}
	return MStatus::kSuccess;
};
MStatus mt_MayaRenderer::translateCamera(const MUuid& id, const MObject& node)
{
	Logging::debug("translateCamera");
	MFnDependencyNode depFn(node);
	MString camName = depFn.name();	
	float imageAspect = (float)width / (float)height;
	float horizontalFilmAperture = getFloatAttr("horizontalFilmAperture", depFn, 24.892f);
	float verticalFilmAperture = getFloatAttr("verticalFilmAperture", depFn, 18.669f);
	float focalLength = getFloatAttr("focalLength", depFn, 35.0f);
	float focusDistance = getFloatAttr("focusDistance", depFn, 10.0f);
	float fStop = getFloatAttr("fStop", depFn, 1000.0f);

	// maya aperture is given in inces so convert to cm and convert to meters
	horizontalFilmAperture = horizontalFilmAperture * 2.54f * 0.01f;
	verticalFilmAperture = verticalFilmAperture * 2.54f * 0.01f;
	verticalFilmAperture = horizontalFilmAperture / imageAspect;
	MString filmBack = MString("") + horizontalFilmAperture + " " + verticalFilmAperture;
	MString focalLen = MString("") + focalLength * 0.001f;

	IdNameStruct idName;
	idName.id = id;
	idName.name = camName + "_" + id;
	idName.mobject = node;
	objectArray.push_back(idName);
	lastShapeId = id;
	return MStatus::kSuccess;
};

MStatus mt_MayaRenderer::translateEnvironment(const MUuid& id, EnvironmentType type)
{
	Logging::debug("translateEnvironment");
	IdNameStruct idName;
	idName.id = id;
	idName.name = MString("Environment");
	idName.mobject = MObject::kNullObj;
	objectArray.push_back(idName);
	return MStatus::kSuccess;
};

MStatus mt_MayaRenderer::translateTransform(const MUuid& id, const MUuid& childId, const MMatrix& matrix)
{
	Logging::debug(MString("translateTransform id: ") + id + " childId " + childId);
	MObject shapeNode;
	IdNameStruct idNameObj;
	for (auto idobj : objectArray)
	{
		if (idobj.id == lastShapeId)
		{
			Logging::debug(MString("Found id object for transform: ") + idobj.name);
			shapeNode = idobj.mobject;
			idNameObj = idobj;
		}
	}
	MString elementInstName = idNameObj.name + "_instance";
	MString elementName = idNameObj.name;
	MMatrix mayaMatrix = matrix;

	if (idNameObj.mobject.hasFn(MFn::kMesh))
	{
	}

	if (idNameObj.mobject.hasFn(MFn::kCamera))
	{
	}

	if (idNameObj.mobject.hasFn(MFn::kAreaLight))
	{
	}


	IdNameStruct idName;
	idName.id = id;
	idName.name = elementInstName;
	idName.mobject = shapeNode;
	objectArray.push_back(idName);

	return MStatus::kSuccess;
};

MStatus mt_MayaRenderer::translateShader(const MUuid& id, const MObject& node)
{
	MObject sgNode;
	// at the moment we only support surface shaders connected to a shadingEngine
	if (node.hasFn(MFn::kShadingEngine))
	{
		sgNode = node;
	}
	else{
		MPlugArray pa, paOut;
		MFnDependencyNode depFn(node);
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
						sgNode = paOut[k].node();
						break;
					}
				}
			}
		}
	}

	MObject surfaceShaderNode = getConnectedInNode(sgNode, "surfaceShader");
	MString surfaceShaderName = getObjectName(surfaceShaderNode);
	MString shadingGroupName = getObjectName(sgNode);
	MString shaderGroupName = shadingGroupName + "_OSLShadingGroup";

	IdNameStruct idName;
	idName.id = id;
	idName.name = shadingGroupName;
	idName.mobject = node;
	objectArray.push_back(idName);
	lastMaterialName = shadingGroupName;
	return MStatus::kSuccess;
};

MStatus mt_MayaRenderer::setProperty(const MUuid& id, const MString& name, bool value)
{
	Logging::debug(MString("setProperty bool: ") + name + " " + value);
	return MStatus::kSuccess;
};
MStatus mt_MayaRenderer::setProperty(const MUuid& id, const MString& name, int value)
{
	Logging::debug(MString("setProperty int: ") + name + " " + value);
	return MStatus::kSuccess;
};
MStatus mt_MayaRenderer::setProperty(const MUuid& id, const MString& name, float value)
{
	Logging::debug(MString("setProperty float: ") + name + " " + value);
	return MStatus::kSuccess;
};
MStatus mt_MayaRenderer::setProperty(const MUuid& id, const MString& name, const MString& value)
{
	Logging::debug(MString("setProperty string: ") + name + " " + value);

	IdNameStruct idNameObj;
	for (auto idobj : objectArray)
	{
		if (idobj.id == id)
		{
			Logging::debug(MString("Found id object for string property: ") + idobj.name);
			if (idobj.name == "Environment")
			{
				if (name == "imageFile")
				{
					Logging::debug(MString("Setting environment image file to: ") + value);
					MString imageFile = value;
					if (value.length() == 0)
					{
						MString mayaRoot = getenv("MAYA_LOCATION");
						imageFile = mayaRoot + "/presets/Assets/IBL/black.exr";
					}
				}
			}
		}
	}

	return MStatus::kSuccess;
};

MStatus mt_MayaRenderer::setShader(const MUuid& id, const MUuid& shaderId)
{
	Logging::debug("setShader");
	IdNameStruct objElement, shaderElement;
	for (auto element : objectArray)
	{
		Logging::debug(MString("Search for obj id: ") + id + " in " + element.id + " name: " + element.name);
		if (element.id == id)
		{
			objElement = element;
			break;
		}
	}
	for (auto element : objectArray)
	{
		Logging::debug(MString("Search for shader id: ") + shaderId + " in " + element.id + " name: " + element.name);
		if (element.id == shaderId)
		{
			shaderElement = element;
			break;
		}
	}
	if ((objElement.mobject == MObject::kNullObj) || (shaderElement.mobject == MObject::kNullObj))
	{
		Logging::error(MString("Unable to find obj or shader for assignment. ShaderName: ") + shaderElement.name + " obj name " + objElement.name);
		return MS::kFailure;
	}
	Logging::debug(MString("--------- Assign shader ") + shaderElement.name + " to object named " + objElement.name);

	return MStatus::kSuccess;
};
MStatus mt_MayaRenderer::setResolution(unsigned int w, unsigned int h)
{
	Logging::debug(MString("setResolution to") + w + " " + h);
	this->width = w;
	this->height = h;
	// Update resolution buffer
	this->rb = (float*)realloc(this->rb, w*h*kNumChannels*sizeof(float));

	for (uint x = 0; x < width; x++)
	{
		for (uint y = 0; y < height; y++)
		{
			uint index = (y * width + x) * kNumChannels;
			rb[index + 0] = x / float(width);
			rb[index + 1] = 0.0f;
			rb[index + 2] = y / float(height);
			rb[index + 3] = 1.0f;
		}
	}

	MString res = MString("") + width + " " + height;
	MString tileString = MString("") + tileSize + " " + tileSize;

	return MStatus::kSuccess;
};
MStatus mt_MayaRenderer::endSceneUpdate()
{
	Logging::debug("endSceneUpdate");
	ProgressParams progressParams;
	progressParams.progress = 0.0;
	progress(progressParams);

	if (asyncStarted)
	{
		renderThread = std::thread(startRenderThread, this);
	}
	else{
		ProgressParams progressParams;
		progressParams.progress = 2.0f;
		progress(progressParams);
	}
	return MStatus::kSuccess;
};
MStatus mt_MayaRenderer::destroyScene()
{
	Logging::debug("destroyScene");
	if (renderThread.joinable())
		renderThread.join();
	ProgressParams progressParams;
	progressParams.progress = -1.0f;
	progress(progressParams);
	return MStatus::kSuccess;
};

bool mt_MayaRenderer::isSafeToUnload()
{
	Logging::debug("isSafeToUnload");
	return true;
};


void mt_MayaRenderer::copyFrameToBuffer(float *frame, int w, int h)
{
	if ((w != width) || (h != height))
	{
		Logging::error("wh ungleich.");
		return;
	}
	int numBytes = width * height * kNumChannels * sizeof(float);
	memcpy(rb, frame, numBytes);

	refreshParams.bottom = 0;
	refreshParams.top = height - 1;
	refreshParams.bytesPerChannel = sizeof(float);
	refreshParams.channels = kNumChannels;
	refreshParams.left = 0;
	refreshParams.right = width - 1;
	refreshParams.width = width;
	refreshParams.height = height;
	refreshParams.data = rb;
	refresh(refreshParams);

}

#endif
