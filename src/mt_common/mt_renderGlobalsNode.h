#ifndef MTAP_GLOBALS_H
#define MTAP_GLOBALS_H

#include "mayarendernodes/renderGlobalsNode.h"

const MString MayaToGlobalsName = "mtGlobals";

class MayaToGlobals : public MayaRenderGlobalsNode
{
public:
						MayaToGlobals();
	virtual				~MayaToGlobals(); 

	static  void*		creator();
	static  MStatus		initialize();

	static	MTypeId		id;

private:

};

#endif