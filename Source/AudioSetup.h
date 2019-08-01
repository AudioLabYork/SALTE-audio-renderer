#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class AudioSetup    :	public Component,
						public ChangeListener,
						private Timer
{
public:
    AudioSetup();
    ~AudioSetup();

    void paint (Graphics&) override;
    void resized() override;

private:

	void changeListenerCallback(ChangeBroadcaster*) override;
	static String getListOfActiveBits(const BigInteger& b);
	void timerCallback() override;
	void dumpDeviceInfo();
	void logMessage(const String& m);

	Random random;
	AudioDeviceSelectorComponent audioSetupComp;
	Label cpuUsageLabel;
	Label cpuUsageText;
	TextEditor diagnosticsBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSetup)
};
