#ifndef MTAP_NEWSWATCHRENDER_H
#define MTAP_NEWSWATCHRENDER_H

#include <maya/MObject.h>

class NewSwatchRenderer;

class MtSwatchRenderer
{
public:
	MtSwatchRenderer();
	~MtSwatchRenderer();
	void mainLoop();
	void setSize(int size);
	void setShader(MObject shader);
	void renderSwatch();
	void renderSwatch(NewSwatchRenderer *sr);
	void fillSwatch(float *pixels);
	bool terminateLoop = false;
	bool enableSwatchRenderer = true;
	bool loopDone = false;
	
	static void startMtSwatchRender(MtSwatchRenderer *swRend);
	static void terminateMtSwatchRender(MtSwatchRenderer *swRend);

	void defineMaterial(MObject shadingNode);

};


#endif