#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class HeadphoneCompensation : public Component
{
public:
	HeadphoneCompensation();
	~HeadphoneCompensation();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill);
	void releaseResources();

	void paint(Graphics& g);
	void resized();

private:

};