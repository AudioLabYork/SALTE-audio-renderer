#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class BinauralHeadView
	: public Component
	, public OpenGLRenderer
{
public:
	BinauralHeadView();
	void newOpenGLContextCreated() override;
	void paint(Graphics& g);
	void renderOpenGL() override;
	void openGLContextClosing() override;

private:
	OpenGLContext m_renderingContext;
};
