#include "BinauralHeadView.h"

BinauralHeadView::BinauralHeadView()
	: frameCounter(0)
{
	setOpaque(true);

	m_renderingContext.setRenderer(this);
	m_renderingContext.attachTo(*this);
	m_renderingContext.setContinuousRepainting(true);
}

void BinauralHeadView::init()
{

}

void BinauralHeadView::deinit()
{

}

void BinauralHeadView::setHeadOrientation(float roll, float pitch, float yaw)
{

}

void BinauralHeadView::newOpenGLContextCreated()
{
	//Colour bgnd = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
	//glClearColor(bgnd.getFloatRed(), bgnd.getFloatGreen(), bgnd.getFloatBlue(), bgnd.getFloatAlpha());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	createShaders();
}

void BinauralHeadView::paint(Graphics& g)
{
	
}

void BinauralHeadView::renderOpenGL()
{
	jassert(OpenGLHelpers::isContextActive());

	auto desktopScale = (float)m_renderingContext.getRenderingScale();

	OpenGLHelpers::clear(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));

	shader->use();

	if (uniforms->projectionMatrix.get() != nullptr)
		uniforms->projectionMatrix->setMatrix4(getProjectionMatrix().mat, 1, false);

	if (uniforms->viewMatrix.get() != nullptr)
		uniforms->viewMatrix->setMatrix4(getViewMatrix().mat, 1, false);

	if (uniforms->lightPosition.get() != nullptr)
		uniforms->lightPosition->set(-15.0f, 10.0f, 15.0f, 0.0f);

	shape->draw(m_renderingContext, *attributes);

	// Reset the element buffers so child Components draw correctly
	m_renderingContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
	m_renderingContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	frameCounter++;
}

void BinauralHeadView::openGLContextClosing()
{
	shader.reset();
	shape.reset();
	attributes.reset();
	uniforms.reset();
}
