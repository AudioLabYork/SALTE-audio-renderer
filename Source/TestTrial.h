#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct Reference
{
	Reference()
		: rendereringOrder(0)
		, gain(1.0f)
	{
	}

	String name;
	String filepath;
	int rendereringOrder;
	float gain;
	String ambixConfig;
};

struct Condition
{
	Condition()
		: score(0)
		, rendereringOrder(0)
		, gain(1.0f)
	{
	}

	String name;
	String filepath;
	float score;
	int rendereringOrder;
	float gain;
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

	void addCondition(Condition* condition);
	void addReference(Reference* reference);

	void randomiseConditions();

	Condition* getCondition(const int index);
	Reference* getReference(const int index);
	
	int getNumberOfConditions() const;
	bool isReferencePresent();

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

	OwnedArray<Reference> m_references;
	OwnedArray<Condition> m_conditions;

	StringArray m_ratingOptions;

	double lastPlaybackHeadPosition;
	bool isLooping;
	float loopStartTime;
	float loopEndTime;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestTrial)
};
