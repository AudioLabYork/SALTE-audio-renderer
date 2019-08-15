#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>
#include <random>

#include "StimulusPlayer.h"

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
	// results vector needs to be added
	double lastPlaybackHeadPosition = 0.0f;
	bool isLooping = true;
	float loopStartTime = 0.0f, loopEndTime = 0.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestTrial)
};

class TC_TS26259 : public Component,
					private Button::Listener,
					private Slider::Listener
{
public:
	TC_TS26259();
	~TC_TS26259();

	void init(StimulusPlayer* player);

	void paint(Graphics&) override;
	void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void sliderValueChanged(Slider* sliderThatWasChanged) override;


private:

	StimulusPlayer* m_player;

	// OSCSender sender;
	TextButton playButton, stopButton, loopButton;
	TextButton selectAButton, selectBButton;
	TextButton prevTrialButton, nextTrialButton;
	OwnedArray<Slider> ratingSliderArray;
	OwnedArray<TestTrial> testTrialArray;
	int currentTrialIndex;
	
	void loadTrial(int trialIndex);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TC_TS26259)
};
