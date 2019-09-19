#include "TestTrial.h"

TestTrial::TestTrial()
	: m_trialId(0)
	, lastPlaybackHeadPosition(0.0f)
	, isLooping(true)
	, loopStartTime(0.0f)
	, loopEndTime(0.0f)
{
}

void TestTrial::init(const String& trialId)
{
	m_trialId = trialId;
}

String TestTrial::getId() const
{
	return m_trialId;
}

MushraCondition* TestTrial::getMCondition(const int index)
{
	return m_MConditions[index];
}

MushraReference* TestTrial::getMReference(const int index)
{
	return m_MReferences[index];
}

TS26259Attribute* TestTrial::getTAttribute(const int index)
{
	return m_TAttributes[index];
}

TS26259Condition* TestTrial::getTCondition(const int index)
{
	return m_TConditions[index];
}

void TestTrial::setTrialName(const String& name)
{
	m_trialName = name;
}

String TestTrial::getTrialName() const
{
	return m_trialName;
}

void TestTrial::setTrialInstruction(const String& instruction)
{
	m_trialInstruction = instruction;
}

String TestTrial::getTrialInstruction() const
{
	return m_trialInstruction;
}

void TestTrial::addMCondition(MushraCondition* condition)
{
	m_MConditions.add(condition);
}

void TestTrial::addMReference(MushraReference* reference)
{
	m_MReferences.add(reference);
}

void TestTrial::addTAttribute(TS26259Attribute* attribute)
{
	m_TAttributes.add(attribute);
}

void TestTrial::addTCondition(TS26259Condition* condition)
{
	m_TConditions.add(condition);
}

void TestTrial::randomiseMConditions()
{
	std::random_shuffle(m_MConditions.begin(), m_MConditions.end());
}

int TestTrial::getNumberOfMConditions() const
{
	return m_MConditions.size();
}

int TestTrial::getNumberOfTAttributes() const
{
	return m_TAttributes.size();
}

bool TestTrial::areTConditionsPresent()
{
	if (m_TConditions.size() == 2)
		return true;
	else
		return false;
}


void TestTrial::setRatingOptions(const StringArray ratings)
{
	m_ratingOptions = ratings;
}

StringArray TestTrial::getRatingOptions() const
{
	return m_ratingOptions;
}

void TestTrial::setLastPlaybackHeadPosition(double time)
{
	lastPlaybackHeadPosition = time;
}

double TestTrial::getLastPlaybackHeadPosition() const
{
	return lastPlaybackHeadPosition;
}

bool TestTrial::getLoopingState() const
{
	return isLooping;
}

void TestTrial::setLooping(bool looping)
{
	isLooping = looping;
}

void TestTrial::setLoopStart(const float startTime)
{
	loopStartTime = startTime;
}

float TestTrial::getLoopStart() const
{
	return loopStartTime;
}

void TestTrial::setLoopEnd(const float endTime)
{
	loopEndTime = endTime;
}

float TestTrial::getLoopEnd() const
{
	return loopEndTime;
}

bool TestTrial::isMReferencePresent()
{
	return !m_MReferences.isEmpty();
}
