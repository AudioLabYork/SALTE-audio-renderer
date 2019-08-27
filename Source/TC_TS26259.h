#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>
#include <random>
#include "TestTrial.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRendererView.h"

using std::vector;

class TC_TS26259 :	public Component,
					public OSCReceiver,
					public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>,
					private Button::Listener,
					private Slider::Listener,
					private ChangeListener
{
public:
	TC_TS26259();
	~TC_TS26259();

	void init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRendererView* rendererView);

private:
	TextButton playButton, stopButton, loopButton;
	TextButton selectAButton, selectBButton;
	TextButton prevTrialButton, nextTrialButton;
	TextButton close;

	OwnedArray<Slider> ratingSliderArray;
	bool timeSyncPlayback = true;

	OscTransceiver* m_oscTxRx;
	StimulusPlayer* m_player;
	BinauralRendererView* m_rendererView;

	OwnedArray<TestTrial> testTrialArray;
	int currentTrialIndex = 0;

	// METHODS
	void paint(Graphics&) override;
	void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void sliderValueChanged(Slider* sliderThatWasChanged) override;
	void oscMessageReceived(const OSCMessage& message) override;
	void changeListenerCallback(ChangeBroadcaster* source) override;
	void loadTrial(int trialIndex);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TC_TS26259)
};
