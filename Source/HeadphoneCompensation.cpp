#include "HeadphoneCompensation.h"

HeadphoneCompensation::HeadphoneCompensation()
{

}

HeadphoneCompensation::~HeadphoneCompensation()
{

}

void HeadphoneCompensation::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{

}

void HeadphoneCompensation::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{

}

void HeadphoneCompensation::releaseResources()
{

}

void HeadphoneCompensation::paint(Graphics& g)
{
	// BACKGROUND
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	// RECTANGULAR OUTLINE
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);
}

void HeadphoneCompensation::resized()
{

}