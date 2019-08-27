#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>
#include <random>
#include "TestTrial.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRendererView.h"

class MushraComponent : public Component,
						public OSCReceiver,
						public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>,
						private Button::Listener,
						private Slider::Listener,
						private ChangeListener
{
public:
	//==============================================================================
	MushraComponent();
	~MushraComponent();

	void init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRendererView* rendererView);

private:
	TextButton playButton, stopButton, loopButton;
	TextButton prevTrialButton, nextTrialButton;
	TextButton selectReferenceButton;
	TextButton close;
	OwnedArray<TextButton> selectConditionButtonArray;
	OwnedArray<Slider> ratingSliderArray;
	bool timeSyncPlayback = true;

	OscTransceiver* m_oscTxRx;
	StimulusPlayer* m_player;
	BinauralRendererView* m_rendererView;

	OwnedArray<TestTrial> testTrialArray;
	int currentTrialIndex = 0;

	// METHODS
	void loadTrial(int trialIndex);
	void paint(Graphics&) override;
	void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void sliderValueChanged(Slider* sliderThatWasChanged) override;
	void oscMessageReceived(const OSCMessage& message) override;
	void changeListenerCallback(ChangeBroadcaster* source) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MushraComponent)
};
