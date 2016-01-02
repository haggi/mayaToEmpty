#include "mt_MayaObject.h"
#include <maya/MPlugArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPathArray.h>

#include "utilities/logging.h"
#include "utilities/tools.h"
//#include "mt_mayaScene.h"

#include "world.h"

static Logging logger;

mt_ObjectAttributes::mt_ObjectAttributes()
{
	needsOwnAssembly = false;
	objectMatrix.setToIdentity();
	this->assemblyObject = nullptr;
}

mt_ObjectAttributes::mt_ObjectAttributes(std::shared_ptr<ObjectAttributes> otherAttr)
{
	std::shared_ptr<mt_ObjectAttributes> other = std::static_pointer_cast<mt_ObjectAttributes>(otherAttr);

	this->hasInstancerConnection = false;
	objectMatrix.setToIdentity();
	this->assemblyObject = nullptr;

	if (other)
	{
		hasInstancerConnection = other->hasInstancerConnection;
		objectMatrix = other->objectMatrix;
		assemblyObject = other->assemblyObject;
	}
};


mt_MayaObject::mt_MayaObject(MObject& mobject) : MayaObject(mobject)
{}

mt_MayaObject::mt_MayaObject(MDagPath& mobject) : MayaObject(mobject)
{}

mt_MayaObject::~mt_MayaObject()
{}


bool mt_MayaObject::geometryShapeSupported()
{
	MFn::Type type = this->mobject.apiType();
	if(this->mobject.hasFn(MFn::kMesh))
		return true;

	if(this->isLight())
		return true;

	if(this->isCamera())
		return true;

	return false;
}

//
//	The purpose of this method is to compare object attributes and inherit them if appropriate.
//	e.g. lets say we assign a color to the top node of a hierarchy. Then all child nodes will be
//	called and this method is used. 
//
std::shared_ptr<ObjectAttributes>mt_MayaObject::getObjectAttributes(std::shared_ptr<ObjectAttributes> parentAttributes)
{
	std::shared_ptr<mt_ObjectAttributes> myAttributes = std::shared_ptr<mt_ObjectAttributes>(new mt_ObjectAttributes(parentAttributes));

	if( this->hasInstancerConnection)
	{
		myAttributes->hasInstancerConnection = true;
	}

	if( this->isGeo())
	{
	}
	
	if( this->isTransform())
	{
		MFnDagNode objNode(this->mobject);
		myAttributes->objectMatrix = objNode.transformationMatrix() * myAttributes->objectMatrix;
	}

	if( myAttributes->hasInstancerConnection)
	{
		myAttributes->objectMatrix.setToIdentity();
	}

	this->attributes = myAttributes;
	return myAttributes;
}


