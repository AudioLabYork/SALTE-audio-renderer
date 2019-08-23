#pragma once

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

	void setReferenceFilepath(String filepath) { referenceFilepath = filepath; }
	String getReferenceFilepath() { return referenceFilepath; }
	bool isReferencePresent() { if (referenceFilepath != "") { return true; } else { return false; } }

private:
	bool referencePresent;
	String referenceFilepath;
	Array<String> filepathArray;
	Array<float> stimulusGainArray;
	Array<float> scoresArray;


	String screenMessage;
	double lastPlaybackHeadPosition = 0.0f;
	bool isLooping = true;
	float loopStartTime = 0.0f, loopEndTime = 0.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestTrial)
};
