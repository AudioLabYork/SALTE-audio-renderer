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
    AudioSetup(AudioDeviceManager& deviceManager);
    ~AudioSetup();

    void paint (Graphics&) override;
    void resized() override;

	bool m_shouldBeVisible = false;

private:

	void changeListenerCallback(ChangeBroadcaster*) override;
	static String getListOfActiveBits(const BigInteger& b);
	void timerCallback() override;
	void dumpDeviceInfo();
	void logMessage(const String& m);

	AudioDeviceManager* admPointer;

	Random random;
	AudioDeviceSelectorComponent audioSetupComp;
	Label cpuUsageLabel;
	Label cpuUsageText;
	TextEditor diagnosticsBox;
	TextButton closeWindowBtn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSetup)
};
