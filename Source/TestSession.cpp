#include "TestSession.h"

TestSession::TestSession()
	: m_sessionId(0)
	, m_subjectData(nullptr)
	, m_startTimeOfTest(0)
	, m_currentTrialIndex(0)
{

}

void TestSession::init(const String& sessionId)
{
	m_sessionId = sessionId;
}

void TestSession::reset()
{
	m_subjectData.reset();
	m_testTrials.clear();
	m_startTimeOfTest = 0;
	m_currentTrialIndex = 0;
}

String TestSession::getId() const
{
	return m_sessionId;
}

void TestSession::setSubjectData(std::unique_ptr<SubjectData> subjectData)
{
	m_subjectData = std::move(subjectData);
}

void TestSession::loadSession(const File& sessionFile)
{
	if (sessionFile.existsAsFile())
	{
		var json = JSON::parse(sessionFile);

		int trials = json.getProperty("numberoftrials", "");

		for (int i = 0; i < trials; ++i)
		{
			var object = json.getProperty("trial" + String(i), "");

			m_testTrials.add(new TestTrial);
			m_testTrials[i]->init(object.getProperty("id", ""));
			m_testTrials[i]->setTrialName(object.getProperty("name", ""));

			File sceneFolder = object.getProperty("scenefolder", "");

			if (sceneFolder.exists())
			{
				if (auto referenceStimuli = object.getProperty("referencestimuli", "").getArray())
				{
					for (auto referenceStimulus : *referenceStimuli)
					{
						File reference = sceneFolder.getFullPathName() + File::getSeparatorString() + referenceStimulus.getProperty("source", "");

						if (reference.existsAsFile())
						{
							Reference* ref = new Reference;
							ref->name = referenceStimulus.getProperty("name", "");
							ref->filepath = reference.getFullPathName();
							m_testTrials[i]->addReference(ref);
						}
					}
				}

				if (auto stimuli = object.getProperty("teststimuli", "").getArray())
				{
					for (auto stimulus : *stimuli)
					{
						Condition* con = new Condition;

						File source = sceneFolder.getFullPathName() + File::getSeparatorString() + stimulus.getProperty("source", "");

						con->name = stimulus.getProperty("name", "");

						if (source.exists())
							con->filepath = source.getFullPathName();

						con->rendereringOrder = stimulus.getProperty("order", "");
						con->ambixConfig = stimulus.getProperty("ambixconfig", "");
						con->score = 0.0f;

						m_testTrials[i]->addCondition(con);
					}
				}
			}

			if (auto ratings = object.getProperty("ratings", "").getArray())
				m_testTrials[i]->setRatingOptions(*ratings);
		}
	}
}

void TestSession::setExportFile(const File& exportFile)
{
	m_exportFile = exportFile;
}

void TestSession::exportResults()
{
	if (m_exportFile.exists())
	{
		FileOutputStream fos(m_exportFile.getFullPathName());

		for (auto& testTrial : m_testTrials)
		{
			for (int i = 0; i < testTrial->getNumberOfConditions(); ++i)
			{
				fos << m_sessionId << ",";

				if (m_subjectData != nullptr)
					fos << m_subjectData->m_id << "," << m_subjectData->m_name << "," << m_subjectData->m_age << "," << m_subjectData->m_gender << ",";
				else
					fos << "-,-,-,-,";

				fos << testTrial->getId() << "," << testTrial->getCondition(i)->name << "," << testTrial->getCondition(i)->score << "\n";
			}
		}
	}
}

void TestSession::setCurrentTrialIndex(const int currentTrialIndex)
{
	m_currentTrialIndex = currentTrialIndex;
}

int TestSession::getCurrentTrialIndex() const
{
	return m_currentTrialIndex;
}

int TestSession::getNumberOfTrials() const
{
	return m_testTrials.size();
}

TestTrial* TestSession::getTrial(const int index)
{
	return m_testTrials[index];
}

void TestSession::begin()
{
	m_startTimeOfTest = Time::getCurrentTime().currentTimeMillis();
}
