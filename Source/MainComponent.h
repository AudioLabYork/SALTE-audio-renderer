#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioSetup.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRenderer.h"
#include "BinauralRendererView.h"
#include "TC_MUSHRA.h"
#include "TestSession.h"
#include "TestSessionForm.h"

class MainComponent
	: public AudioAppComponent
	, public Button::Listener
	, public TestSessionForm::Listener
	, public MushraComponent::Listener
	, public ChangeListener
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
	void formCompleted() override;
	void testCompleted() override;
    void changeListenerCallback(ChangeBroadcaster* source) override;

private:
	AudioSetup as;
	OscTransceiver oscTxRx;
    StimulusPlayer sp;
	BinauralRenderer br;
	BinauralRendererView brv;
	MushraComponent mc;

	TestSession m_testSession;
	TestSessionForm m_testSessionForm;

	AudioBuffer<float> processBuffer;
	int m_maxSamplesPerBlock;

	String logWindowMessage;
	TextEditor logWindow;

	TextButton openAudioDeviceManager, connectOscButton;
	Label clientTxIpLabel, clientTxPortLabel, clientRxPortLabel;

	// save and load settings
	File settingsFile;
	void loadSettings();
	void saveSettings();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
