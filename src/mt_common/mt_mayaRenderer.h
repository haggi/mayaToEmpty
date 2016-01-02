#ifndef MAYA_RENDERER_H
#define MAYA_RENDERER_H

#include <maya/MTypes.h>
#include <map>
#include <thread>
#include <mutex>
#include <vector>

#if MAYA_API_VERSION >= 201600
#include <maya/MPxRenderer.h>

struct IdNameStruct{
	MUuid id;
	MString name; // in appleseed objects must have unique names
	MObject mobject; // I need to know if I have a light or a mesh or a camera
};

class mt_MayaRenderer : public MPxRenderer
{
public:
	RefreshParams refreshParams;
	float* rb = nullptr;
	int tileSize = 32;
	int initialSize = 256;
	mt_MayaRenderer();
	virtual ~mt_MayaRenderer();
	static void* creator();
	virtual MStatus startAsync(const JobParams& params);
	virtual MStatus stopAsync();
	virtual bool isRunningAsync();
	void initProject();
	void initEnv();
	virtual MStatus beginSceneUpdate();

	virtual MStatus translateMesh(const MUuid& id, const MObject& node);
	virtual MStatus translateLightSource(const MUuid& id, const MObject& node);
	virtual MStatus translateCamera(const MUuid& id, const MObject& node);
	virtual MStatus translateEnvironment(const MUuid& id, EnvironmentType type);
	virtual MStatus translateTransform(const MUuid& id, const MUuid& childId, const MMatrix& matrix);
	virtual MStatus translateShader(const MUuid& id, const MObject& node);

	virtual MStatus setProperty(const MUuid& id, const MString& name, bool value);
	virtual MStatus setProperty(const MUuid& id, const MString& name, int value);
	virtual MStatus setProperty(const MUuid& id, const MString& name, float value);
	virtual MStatus setProperty(const MUuid& id, const MString& name, const MString& value);

	virtual MStatus setShader(const MUuid& id, const MUuid& shaderId);
	virtual MStatus setResolution(unsigned int width, unsigned int height);

	virtual MStatus endSceneUpdate();

	virtual MStatus destroyScene();

	virtual bool isSafeToUnload();

	void copyFrameToBuffer(float *frame, int w, int h);
	void render();

private:
	int width, height;
	//Render output buffer, it is R32G32B32A32_FLOAT format.
	std::thread renderThread;
	MUuid lastShapeId; // save the last shape id, needed by translateTransform
	MString lastMaterialName = "default";
	std::vector<IdNameStruct> objectArray;
	bool asyncStarted = false;
};
#endif

#endif