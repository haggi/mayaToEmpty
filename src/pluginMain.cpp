#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>
#include <maya/MStatus.h>
#include <maya/MDrawRegistry.h>
#include <maya/MSwatchRenderRegister.h>

#include "mayato.h"
#include "mt_common/mt_renderGlobalsNode.h"
#include "swatchesRenderer/swatchRenderer.h"
#include "swatchesRenderer/NewSwatchRenderer.h"

#include "utilities/tools.h"
#include "threads/renderQueueWorker.h"
#include "world.h"
#include "mt/version.h"

#if MAYA_API_VERSION >= 201600
#include "mt_common/mt_mayaRenderer.h"
#endif

static const MString swatchName("MayaToRenderSwatch");
static const MString swatchFullName("swatch/MayaToRenderSwatch");

static const MString asDisneyMaterialId("asDisneyMaterialId");
static const MString asDisneyMaterialIdDrawDBClassification("drawdb/shader/surface/asDisneyMaterialId");
static const MString asDisneyMaterialIdFullClassification("shader/surface:MayaTo/material:" + swatchFullName + ":" + asDisneyMaterialIdDrawDBClassification);

MStatus initializePlugin( MObject obj )
{
	for (auto versionElement : getFullVersionString())
		MGlobal::displayInfo(versionElement.c_str());

	MStatus   status;
	MFnPlugin plugin( obj, VENDOR, getFullVersionString()[0].c_str(), "Any");

	status = plugin.registerCommand(MAYATOCMDNAME, MayaToCmd::creator, MayaToCmd::newSyntax);
	if (!status) {
		status.perror("cannot register command: mayato");
		return status;
	}

	status = plugin.registerNode(MayaToGlobalsName, MayaToGlobals::id, MayaToGlobals::creator, MayaToGlobals::initialize);
	if (!status) {
		status.perror("cannot register node: MayaToGlobals");
		return status;
	}

	//MString command( "if( `window -exists createRenderNodeWindow` ) {refreshCreateRenderNodeWindow(\"" );
	//command += UserClassify;
	//command += "\");}\n";
	//MGlobal::executeCommand( command );

	setRendererName("MayaTo");
	setRendererShortCutName("mt");
	setRendererHome(getenv("mt_HOME"));

	MString cmd = MString("import MayaTo.mt_initialize as minit; minit.initRenderer()");
	MGlobal::displayInfo("try to register...");
	status = MGlobal::executePythonCommand(cmd, true, false);
	if(!status)
	{
		status.perror("Problem executing cmd: mt_initialize.initRenderer()");
		return status;
	}

	MayaTo::defineWorld();
	MString loadPath = plugin.loadPath();
	MayaTo::getWorldPtr()->shaderSearchPath.append(loadPath);

	if (MGlobal::mayaState() != MGlobal::kBatch)
	{
		MSwatchRenderRegister::registerSwatchRender(swatchName, NewSwatchRenderer::creator);
	}

#if MAYA_API_VERSION >= 201600
	status = plugin.registerRenderer("MayaTo", mt_MayaRenderer::creator);
	if (!status) {
		status.perror("cannot register node: MayaTo Maya renderer");
		return status;
	}
#endif
	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	MayaTo::deleteWorld();
	
	std::cout << "deregister mt cmd\n";
	status = plugin.deregisterCommand( MAYATOCMDNAME );
	if (!status) {
		status.perror("cannot deregister command: MayaToCmd");
		return status;
	}

	if (MGlobal::mayaState() != MGlobal::kBatch)
		MSwatchRenderRegister::unregisterSwatchRender(swatchName);


#if MAYA_API_VERSION >= 201600
	status = plugin.deregisterRenderer("MayaTo");
	if (!status) {
		status.perror("cannot deregister node: Maya renderer");
		return status;
	}
#endif

	std::cout << "deregister mt globals\n";
	status = plugin.deregisterNode( MayaToGlobals::id );
	if (!status) {
		status.perror("cannot deregister node: MayaToGlobals");
		return status;
	}

	std::cout << "minit.unregister()\n";
	MString cmd = MString("import mt_initialize as minit; minit.unregister()");
	status = MGlobal::executePythonCommand(cmd);
	if(!status)
	{
		status.perror("Problem executing cmd: mt_initialize.unregister()");
		return status;
	}

	return status;
}
