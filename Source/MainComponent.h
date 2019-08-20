#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioSetup.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRenderer.h"
#include "BinauralRendererView.h"

// additional test modules
#include "TC_MUSHRA.h"
#include "TC_OSC.h"
#include "TC_TS26259.h"

class MainComponent   :     public AudioAppComponent,
                            private Button::Listener,
                            public ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* buttonThatWasClicked) override;

    // LOG WINDOW UPDATE
    void changeListenerCallback(ChangeBroadcaster* source) override;

private:

	// COMPONENTS
	AudioSetup as;
	OscTransceiver oscTxRx;
    StimulusPlayer sp;
	BinauralRenderer br;
	BinauralRendererView brv;
	OscTestComponent otc; 	// OSC test component
	TC_TS26259 tsc; 	// 3GPP TS 26.259 component
	MushraComponent mc; 	// MUSHRA COMPONENT


	AudioBuffer<float> processBuffer;
	int m_maxSamplesPerBlock;

	String logWindowMessage;
	TextEditor logWindow;

	TextButton openAudioDeviceManager, connectOscButton;
	TextButton loadOSCTestButton, loadMushraBtn, loadLocTestBtn, loadTS126259TestBtn;
	Label clientTxIpLabel, clientTxPortLabel, clientRxPortLabel;

	// save and load settings
	File settingsFile;
	void loadSettings();
	void saveSettings();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
