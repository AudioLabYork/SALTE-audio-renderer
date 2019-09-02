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

Condition* TestTrial::getCondition(const int index)
{
	return m_conditions[index];
}

Reference* TestTrial::getReference(const int index)
{
	return m_references[index];
}

void TestTrial::setTrialName(const String& name)
{
	m_trialName = name;
}

String TestTrial::getTrialName() const
{
	return m_trialName;
}

void TestTrial::addCondition(Condition* condition)
{
	m_conditions.add(condition);
}

void TestTrial::addReference(Reference* reference)
{
	m_references.add(reference);
}

void TestTrial::randomiseConditions()
{
	std::random_shuffle(m_conditions.begin(), m_conditions.end());
}

int TestTrial::getNumberOfConditions() const
{
	return m_conditions.size();
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

bool TestTrial::isReferencePresent()
{
	return !m_references.isEmpty();
}
