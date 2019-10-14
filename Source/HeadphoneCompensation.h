#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class HeadphoneCompensation :	public Component,
								public Button::Listener
{
public:
	HeadphoneCompensation();
	~HeadphoneCompensation();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void processBlock(AudioBuffer<float>& buffer);
	void releaseResources();

	void buttonClicked(Button* buttonClicked) override;

	void paint(Graphics& g);
	void resized();

private:
	TextButton m_firBrowse;
	ToggleButton m_btnEnabled;
	dsp::Convolution m_conv;

	bool m_enableEq;
};