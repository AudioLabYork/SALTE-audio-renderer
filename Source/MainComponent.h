#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioSetup.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "LoudspeakerRenderer.h"
#include "BinauralRenderer.h"
#include "BinauralRendererView.h"
#include "MixedMethods.h"
#include "AuditoryLocalisation.h"
#include "TestSession.h"
#include "TestSessionForm.h"
#include "HeadphoneCompensation.h"

class MainComponent
	: public AudioAppComponent
	, public Button::Listener
	, public TestSessionForm::Listener
	, public MixedMethodsComponent::Listener
	, public ChangeListener
{
public:
	//==============================================================================
	MainComponent();
	~MainComponent();

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	//==============================================================================
	void paint(Graphics& g) override;
	void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void formCompleted() override;
	void testCompleted() override;
	void changeListenerCallback(ChangeBroadcaster* source) override;

private:
	AudioSetup m_audioSetup;
	OscTransceiver oscTxRx;
	StimulusPlayer m_stimulusPlayer;
	LoudspeakerRenderer m_loudspeakerRenderer;
	BinauralRenderer m_binauralRenderer;
	BinauralRendererView m_binauralRendererView;
	MixedMethodsComponent m_mixedMethods;
	AuditoryLocalisation m_localisationComponent;
	HeadphoneCompensation m_headphoneCompensation;

	TestSession m_testSession;
	TestSessionForm m_testSessionForm;

	AudioBuffer<float> processBuffer;
	int m_maxSamplesPerBlock;

	String logWindowMessage;
	TextEditor logWindow;

	TextButton openAudioDeviceManager, connectOscButton;
	Label clientTxIpLabel, clientTxPortLabel, clientRxPortLabel;

	TextButton showMixedComp, showLocComp;
	TextButton showTestInterface;
	bool showOnlyTestInterface;

	ImageComponent imageComponent;

	// save and load settings
	File audioSettingsFile;
	void loadSettings();
	void saveSettings();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
