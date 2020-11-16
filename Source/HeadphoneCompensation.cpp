#include "HeadphoneCompensation.h"

HeadphoneCompensation::HeadphoneCompensation()
	: m_enableEq(false)
{
	m_firBrowse.setButtonText("Select FIR...");
	m_firBrowse.addListener(this);
	addAndMakeVisible(m_firBrowse);

	m_btnEnabled.setButtonText("Enable EQ");
	m_btnEnabled.addListener(this);
	addAndMakeVisible(m_btnEnabled);
}

HeadphoneCompensation::~HeadphoneCompensation()
{
}

void HeadphoneCompensation::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    dsp::ProcessSpec spec{ sampleRate, static_cast<uint32>(samplesPerBlockExpected), 2 };
	m_conv.prepare(spec);
}

void HeadphoneCompensation::processBlock(AudioBuffer<float>& buffer)
{
	if (m_enableEq)
	{
		dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), 2, buffer.getNumSamples());
		m_conv.process(dsp::ProcessContextReplacing<float>(block));
	}
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
	m_btnEnabled.setBounds(10, 45, 150, 30);
}

void HeadphoneCompensation::buttonClicked(Button* buttonClicked)
{
	if (buttonClicked == &m_firBrowse)
	{
#if JUCE_MODAL_LOOPS_PERMITTED
		FileChooser fc("Select Headphone Response file to open...",
			File::getCurrentWorkingDirectory(),
			"*.wav",
			true);

		if (fc.browseForFileToOpen())
		{
			File chosenFile = fc.getResult();
			m_conv.reset();
            m_conv.loadImpulseResponse(chosenFile, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, 0);
		}
#endif
	}
	else if (buttonClicked == &m_btnEnabled)
	{
		m_enableEq = m_btnEnabled.getToggleState();
	}
}
