#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class HeadphoneCompensation :	public Component,
								public Button::Listener
{
public:
	HeadphoneCompensation();
	~HeadphoneCompensation();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill);
	void releaseResources();

	void buttonClicked(Button* buttonClicked) override;

	void paint(Graphics& g);
	void resized();

private:

	TextButton m_firBrowse;

};