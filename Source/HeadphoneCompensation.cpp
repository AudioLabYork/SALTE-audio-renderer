#include "HeadphoneCompensation.h"

HeadphoneCompensation::HeadphoneCompensation()
{

	m_firBrowse.setButtonText("Select FIR...");
	m_firBrowse.addListener(this);
	addAndMakeVisible(m_firBrowse);

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
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);
}

void HeadphoneCompensation::resized()
{
	m_firBrowse.setBounds(10, 10, 150, 30);
}

void HeadphoneCompensation::buttonClicked(Button* buttonClicked)
{
	if (buttonClicked == &m_firBrowse)
	{
		// browse for FIR
	}
}