#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMessageAttribute.h>

#include "mt_renderGlobalsNode.h"

MTypeId	MayaToGlobals::id(0x0011CF40);

MayaToGlobals::MayaToGlobals()
{
	imageFormatList.append("Png");
	imageFormatList.append("Exr");

	filterTypeList.append("Box");
	filterTypeList.append("Blackman-Harris");
	filterTypeList.append("Catmull-Rom");
	filterTypeList.append("Mitchell");
	filterTypeList.append("Gauss");
	filterTypeList.append("Triangle");
}

MayaToGlobals::~MayaToGlobals()
{}

void *MayaToGlobals::creator()
{
	return new MayaToGlobals();
}

MStatus	MayaToGlobals::initialize()
{
	MayaRenderGlobalsNode::initialize();

	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	MFnGenericAttribute gAttr;
	MFnEnumAttribute eAttr;
	MFnMessageAttribute mAttr;
	MStatus stat = MStatus::kSuccess;

	return stat;

}

