#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <random>

// MUSHRA or MUSHA (no reference) condition
struct MushraCondition
{
	MushraCondition()
		: score(0)
		, minScore(0)
		, maxScore(100)
		, renderingOrder(0)
		, gain(1.0f)
	{
	}

	String name;
	float score, minScore, maxScore;
	// stimulus settings
	String filepath;
	int renderingOrder;
	float gain;
	String ambixConfig;
};

// MUSHRA reference
struct MushraReference
{
	MushraReference()
		: renderingOrder(0)
		, gain(1.0f)
	{
	}

	String name;
	// stimulus settings
	String filepath;
	int renderingOrder;
	float gain;
	String ambixConfig;
};

// TS26.259 attribute (typically there are 4 of these in a single trial)
struct TS26259Attribute
{
	TS26259Attribute()
		: score(0)
		, minScore(-3)
		, maxScore(3)
	{
	}

	String name;
	float score, minScore, maxScore;
};

// TS26.259 condition trigger (typically one of the two: A or B, doesn't contain the scores)
struct TS26259Condition
{
	TS26259Condition()
		: gain(1.0f)
		, renderingOrder(0)
	{
	}

	String name;
	// stimulus settings
	String filepath;
	float gain;
	int renderingOrder;
	String ambixConfig;
};


class TestTrial
{
public:
	TestTrial();

	void init(const String& trialId);

	String getId() const;
	
	void setTrialName(const String& name);
	String getTrialName() const;

	void setTrialInstruction(const String& instruction);
	String getTrialInstruction() const;

	void addMCondition(MushraCondition* condition);
	void addMReference(MushraReference* reference);
	void addTAttribute(TS26259Attribute* attribute);
	void addTCondition(TS26259Condition* condition);

	void randomiseMConditions();

	MushraCondition* getMCondition(const int index);
	MushraReference* getMReference(const int index);
	TS26259Attribute* getTAttribute(const int index);
	TS26259Condition* getTCondition(const int index);
	
	int getNumberOfMConditions() const;
	bool isMReferencePresent();
	int getNumberOfTAttributes() const;
	bool areTConditionsPresent();
	void randomiseTConditions();

	void setRatingOptions(const StringArray ratings);
	StringArray getRatingOptions() const;
	
	void setLastPlaybackHeadPosition(double time);
	double getLastPlaybackHeadPosition() const;

	void setLooping(bool looping);
	bool getLoopingState() const;
	
	void setLoopStart(const float startTime);
	float getLoopStart() const;
	
	void setLoopEnd(const float endTime);
	float getLoopEnd() const;

private:
	String m_trialId;
	String m_trialName;
	String m_trialInstruction;

	OwnedArray<MushraCondition> m_MConditions;
	OwnedArray<MushraReference> m_MReferences;
	OwnedArray<TS26259Attribute> m_TAttributes;
	OwnedArray<TS26259Condition> m_TConditions;

	StringArray m_ratingOptions;

	double lastPlaybackHeadPosition;
	bool isLooping;
	float loopStartTime;
	float loopEndTime;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestTrial)
};
