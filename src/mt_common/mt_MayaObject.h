#ifndef MTAP_MAYA_OBJECT_H
#define MTAP_MAYA_OBJECT_H

#include <maya/MMatrix.h>

#include "mayaObject.h"

class mt_MayaObject;

class mt_ObjectAttributes : public ObjectAttributes
{
public:
	mt_ObjectAttributes();
	mt_ObjectAttributes(std::shared_ptr<ObjectAttributes>);
	bool needsOwnAssembly;
	MMatrix objectMatrix;
	MayaObject *assemblyObject; // mayaObject above for which an assembly will be created
};

class mt_MayaObject : public MayaObject
{
public:
	mt_MayaObject(MObject&);
	mt_MayaObject(MDagPath&);
	~mt_MayaObject();

	virtual bool geometryShapeSupported();
	virtual std::shared_ptr<ObjectAttributes> getObjectAttributes(std::shared_ptr<ObjectAttributes> parentAttributes = nullptr);
};

#endif