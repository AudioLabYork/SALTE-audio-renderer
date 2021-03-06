#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "TestTrial.h"
#include "SubjectData.h"

class TestSession
{
public:
	TestSession();

	void begin();
	void reset();

	void randomiseTrials();

	String getId() const;
	void setSubjectData(std::unique_ptr<SubjectData> subjectData);

	void loadSession(const File& sessionFile);
	void setExportFile(const File& exportFile);
	void exportResults();
	
	void setCurrentTrialIndex(const int currentTrialIndex);
	int getCurrentTrialIndex() const;
	int getNumberOfTrials() const;
	TestTrial* getTrial(const int index);

private:
	String m_sessionId;
	String m_sessionName;

	std::unique_ptr<SubjectData> m_subjectData;
	OwnedArray<TestTrial> m_testTrials;
	int64 m_startTimeOfTest;

	int m_currentTrialIndex;

	File m_exportFile;

	bool m_randTrials;
	bool m_randConditions;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestSession)
};
