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
                            public OSCReceiver,
                            public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
							// public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>, // only for ypr debugging
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

    // OSC
    oscTransceiver remoteInterfaceTxRx; // osc object to communicate with the user interface (Unity, iPad, ...)
    void oscMessageReceived(const OSCMessage& message) override;

    // LOG WINDOW UPDATE
    void changeListenerCallback(ChangeBroadcaster* source) override;

private:
	AudioSetup as;
    StimulusPlayer sp;
	BinauralRenderer br;

	AudioBuffer<float> processBuffer;
	int m_maxSamplesPerBlock;

	BinauralRendererView brv;

    String logWindowMessage;
    TextEditor logWindow;
    
    TextButton openAudioDeviceManager;
	TextButton loadOSCTestButton, loadMushraBtn, loadLocTestBtn, loadTS126259TestBtn;
	Label clientTxIpLabel, clientTxPortLabel, clientRxPortLabel;

	// OSC test component
	OscTestComponent otc;

	// 3GPP TS 26.259 component
	TC_TS26259 tsc;

	// MUSHRA COMPONENT
	MushraComponent mc;
	void configureMushra();

	// save and load settings
	File settingsFile;
	void loadSettings();
	void saveSettings();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
