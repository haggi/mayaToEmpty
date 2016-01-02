#ifndef MTAP_CMD_H
#define MTAP_CMD_H

#include <string.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MString.h>

#define MAYATOCMDNAME "mayatocmd"

class  MayaToCmd: public MPxCommand
{
public:
	MayaToCmd();
	virtual			~MayaToCmd();
	static MSyntax	newSyntax();

	MStatus     	doIt( const MArgList& args );
	static void*	creator();
	void setLogLevel();

private:
};

#endif