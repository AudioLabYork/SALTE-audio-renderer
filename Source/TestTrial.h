#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class TestTrial
{
public:
	TestTrial();
	~TestTrial();

	void setFilepath(int fileindex, const String& filepath);
	String getFilepath(int fileindex);
	
	void setScore(int fileindex, float score);
	float getScore(int fileindex);
	
	void setGain(int fileindex, float gainInDB);
	float getGain(int fileindex);

	void setScreenMessage(const String& msg);
	String getScreenMessage();
	
	int getNumberOfConditions();
	
	void setRatingOptions(const StringArray ratings);
	StringArray getRatingOptions();
	
	void setLastPlaybackHeadPosition(double time);
	double getLastPlaybackHeadPosition();
	
	bool getLoopingState();
	void setLooping(bool looping);
	
	void setLoopStart(float startTime);
	float getLoopStart();
	
	void setLoopEnd(float endTime);
	float getLoopEnd();

	void setReferenceFilepath(const String& filepath);
	String getReferenceFilepath();
	
	bool isReferencePresent();

private:
	String referenceFilepath;
	StringArray filepathArray;
	Array<float> stimulusGainArray;
	Array<float> scoresArray;

	String screenMessage;
	StringArray ratingOptions;

	double lastPlaybackHeadPosition = 0.0f;
	bool isLooping = true;
	float loopStartTime = 0.0f;
	float loopEndTime = 0.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestTrial)
};
