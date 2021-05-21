#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioSetup.h"
#include "OscTransceiver.h"
#include "MixedMethods.h"
#include "TestSession.h"
#include "TestSessionForm.h"
#include "AuditoryLocalisation.h"
#include "RendererView.h"
#include "StimulusPlayer.h"
#include "LoudspeakerRenderer.h"
#include "BinauralRenderer.h"
#include "HeadphoneCompensation.h"
#include "OutputRouting.h"

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
	RendererView m_rendererView;
	MixedMethodsComponent m_mixedMethods;
	AuditoryLocalisation m_localisationComponent;
	HeadphoneCompensation m_headphoneCompensation;
	OutputRouting m_lspkRouter;

	TestSession m_testSession;
	TestSessionForm m_testSessionForm;

	AudioBuffer<float> processBuffer;
	int m_maxSamplesPerBlock;

	String logWindowMessage;
	TextEditor logWindow;

	TextButton openAudioDeviceManager, connectOscButton;
	Label clientTxIpLabel, clientTxPortLabel, clientRxPortLabel;
	String lastRemoteIpAddress;
	ToggleButton enableLocalIp;
	TextButton showMixedComp, showLocComp;
	TextButton openRouter;
	TextButton showTestInterface;
	bool showOnlyTestInterface;

	void updateOscSettings(bool keepConnected);

	ImageComponent imageComponent;

	// app settings
	void loadSettings();
	void saveSettings();
	ApplicationProperties appSettings;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
