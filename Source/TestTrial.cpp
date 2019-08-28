#include "TestTrial.h"

TestTrial::TestTrial()
{
}

TestTrial::~TestTrial()
{
}

void TestTrial::setFilepath(int fileindex, const String& filepath)
{
	filepathArray.set(fileindex, filepath);
}

String TestTrial::getFilepath(int fileindex)
{
	return filepathArray[fileindex];
}

void TestTrial::setScore(int fileindex, float score)
{
	scoresArray.set(fileindex, score);
}

float TestTrial::getScore(int fileindex)
{
	return scoresArray[fileindex];
}

void TestTrial::setGain(int fileindex, float gainInDB)
{
	stimulusGainArray.set(fileindex, gainInDB);
}

float TestTrial::getGain(int fileindex)
{
	return stimulusGainArray[fileindex];
}
void TestTrial::setScreenMessage(const String& msg)
{
	screenMessage = msg;
}
String TestTrial::getScreenMessage()
{
	return screenMessage;
}

int TestTrial::getNumberOfConditions()
{
	return filepathArray.size();
}

void TestTrial::setRatingOptions(const StringArray ratings)
{
	ratingOptions = ratings;
}

StringArray TestTrial::getRatingOptions()
{
	return ratingOptions;
}

void TestTrial::setLastPlaybackHeadPosition(double time)
{
	lastPlaybackHeadPosition = time;
}

double TestTrial::getLastPlaybackHeadPosition()
{
	return lastPlaybackHeadPosition;
}

bool TestTrial::getLoopingState()
{
	return isLooping;
}

void TestTrial::setLooping(bool looping)
{
	isLooping = looping;
}

void TestTrial::setLoopStart(float startTime)
{
	loopStartTime = startTime;
}

float TestTrial::getLoopStart()
{
	return loopStartTime;
}

void TestTrial::setLoopEnd(float endTime)
{
	loopEndTime = endTime;
}

float TestTrial::getLoopEnd()
{
	return loopEndTime;
}

void TestTrial::setReferenceFilepath(const String& filepath)
{
	referenceFilepath = filepath;
}

String TestTrial::getReferenceFilepath()
{
	return referenceFilepath;
}

bool TestTrial::isReferencePresent()
{
	return referenceFilepath.isNotEmpty();
}
