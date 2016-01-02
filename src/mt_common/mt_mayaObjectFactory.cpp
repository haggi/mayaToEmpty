#include "mayaObjectFactory.h"
#include "mt_MayaObject.h"

namespace MayaTo
{
	std::shared_ptr<MayaObject> MayaObjectFactory::createMayaObject(MObject& mobject)
	{
		return std::shared_ptr<MayaObject>(new mt_MayaObject(mobject));
	}
	std::shared_ptr<MayaObject> MayaObjectFactory::createMayaObject(MDagPath& objPath)
	{
		return std::shared_ptr<MayaObject>(new mt_MayaObject(objPath));
	}
};