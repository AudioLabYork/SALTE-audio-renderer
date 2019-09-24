#include "BinauralHeadView.h"

BinauralHeadView::BinauralHeadView()
{
	setOpaque(true);

	m_renderingContext.setRenderer(this);
	m_renderingContext.attachTo(*this);
	m_renderingContext.setContinuousRepainting(true);
}

void BinauralHeadView::newOpenGLContextCreated()
{
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
}

void BinauralHeadView::paint(Graphics& g) {}

void BinauralHeadView::renderOpenGL()
{
	auto desktopScale = (float)m_renderingContext.getRenderingScale();
	glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_TRIANGLES);
	glVertex2f(-0.5, -0.5);
	glVertex2f(-0.5, 0.5);
	glVertex2f(0.5, 0.5);
	glEnd();
}

void BinauralHeadView::openGLContextClosing()
{

}
