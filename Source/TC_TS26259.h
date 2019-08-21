#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>
#include <random>
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRendererView.h"

using std::vector;

class TestTrial
{
public:
	TestTrial()
	{
	}
	~TestTrial()
	{
	}

	void setFilepath(int fileindex, String filepath) { filepathArray.set(fileindex, filepath); }
	String getFilepath(int fileindex) { return filepathArray[fileindex]; }
	void setScore(int fileindex, float score) { scoresArray.set(fileindex, score); }
	float getScore(int fileindex) { return scoresArray[fileindex]; }
	void setGain(int fileindex, float gainInDB) { stimulusGainArray.set(fileindex, gainInDB); }
	float getGain(int fileindex) { return stimulusGainArray[fileindex]; }
	void setScreenMessage(String msg) { screenMessage = msg; }
	String getScreenMessage() { return screenMessage; }
	int getNumberOfConditions() { return filepathArray.size(); }
	void setLastPlaybackHeadPosition(double time) { lastPlaybackHeadPosition = time; }
	double getLastPlaybackHeadPosition() { return lastPlaybackHeadPosition; }
	bool getLoopingState() { return isLooping; }
	void setLooping(bool looping) { isLooping = looping; }
	void setLoopStart(float startTime) { loopStartTime = startTime; }
	float getLoopStart() { return loopStartTime; }
	void setLoopEnd(float endTime) { loopEndTime = endTime; }
	float getLoopEnd() { return loopEndTime; }

private:
	Array<String> filepathArray;
	Array<float> stimulusGainArray;
	Array<float> scoresArray;


	String screenMessage;
	double lastPlaybackHeadPosition = 0.0f;
	bool isLooping = true;
	float loopStartTime = 0.0f, loopEndTime = 0.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestTrial)
};

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

	void paint(Graphics&) override;
	void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void sliderValueChanged(Slider* sliderThatWasChanged) override;

	void oscMessageReceived(const OSCMessage& message) override;
	void changeListenerCallback(ChangeBroadcaster* source) override;

private:
	TextButton playButton, stopButton, loopButton;
	TextButton selectAButton, selectBButton;
	TextButton prevTrialButton, nextTrialButton;
	OwnedArray<Slider> ratingSliderArray;
	bool timeSyncPlayback = true;

	OscTransceiver* m_oscTxRx;
	StimulusPlayer* m_player;
	BinauralRendererView* m_rendererView;

	OwnedArray<TestTrial> testTrialArray;
	int currentTrialIndex = 0;

	// vector<vector<float>> scoresMatrix;
	
	void loadTrial(int trialIndex);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TC_TS26259)
};
