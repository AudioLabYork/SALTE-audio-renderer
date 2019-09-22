#include "TestSession.h"

TestSession::TestSession()
	: m_sessionId(0)
	, m_subjectData(nullptr)
	, m_startTimeOfTest(0)
	, m_currentTrialIndex(0)
{

}

void TestSession::reset()
{
	m_subjectData.reset();
	m_testTrials.clear();
	m_startTimeOfTest = 0;
	m_currentTrialIndex = 0;
}

void TestSession::randomiseTrials()
{
	std::random_shuffle(m_testTrials.begin(), m_testTrials.end());
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
		// parse the JSON file
		var json = JSON::parse(sessionFile);

		// get number of trials
		int trials = json.getProperty("numberoftrials", "");

		// load each trial
		for (int i = 0; i < trials; ++i)
		{
			// create trial object
			var trialObject = json.getProperty("trial" + String(i), "");

			m_testTrials.add(new TestTrial);

			// setup the trial settings
			m_testTrials[i]->init(trialObject.getProperty("id", ""));
			m_testTrials[i]->setTrialName(trialObject.getProperty("name", ""));
			m_testTrials[i]->setTrialInstruction(trialObject.getProperty("instruction", ""));
			File sceneFolder(sessionFile.getParentDirectory().getFullPathName() + "/" + trialObject.getProperty("scenefolder", "").toString());
			File ambixconfigFolder(sessionFile.getParentDirectory().getFullPathName() + "/" + trialObject.getProperty("ambixconfigfolder", "").toString());

			if (sceneFolder.exists())
			{
				// MUSHRA REFERENCE
				if (auto referenceStimuli = trialObject.getProperty("MushraReference", "").getArray())
				{
					for (auto referenceStimulus : *referenceStimuli)
					{
						File reference(sceneFolder.getFullPathName() + File::getSeparatorString() + referenceStimulus.getProperty("source", "").toString());
						File ambixconfig(ambixconfigFolder.getFullPathName() + File::getSeparatorString() + referenceStimulus.getProperty("ambixconfig", "").toString());

						if (reference.existsAsFile())
						{
							MushraReference* ref = new MushraReference;
							ref->name = referenceStimulus.getProperty("name", "");
							ref->filepath = reference.getFullPathName();
							ref->renderingOrder = referenceStimulus.getProperty("order", "");
							ref->gain = referenceStimulus.getProperty("gain", "");
							// ref->ambixConfig = referenceStimulus.getProperty("ambixconfig", "");
							if(ambixconfig.existsAsFile()) ref->ambixConfig = ambixconfig.getFullPathName();

							m_testTrials[i]->addMReference(ref);
						}
					}
				}

				// MUSHRA CONDITIONS
				if (auto stimuli = trialObject.getProperty("MushraConditions", "").getArray())
				{
					for (auto stimulus : *stimuli)
					{
						MushraCondition* con = new MushraCondition;

						File source(sceneFolder.getFullPathName() + File::getSeparatorString() + stimulus.getProperty("source", "").toString());
						File ambixconfig(ambixconfigFolder.getFullPathName() + File::getSeparatorString() + stimulus.getProperty("ambixconfig", "").toString());

						con->name = stimulus.getProperty("name", "");

						if (source.exists())
							con->filepath = source.getFullPathName();

						con->score = 0.0f;
						con->renderingOrder = stimulus.getProperty("order", "");
						con->gain = stimulus.getProperty("gain", "");
						if (ambixconfig.existsAsFile()) con->ambixConfig = ambixconfig.getFullPathName();

						m_testTrials[i]->addMCondition(con);
					}

					// randomise MUSHRA conditions
					m_testTrials[i]->randomiseMConditions();
				}

				// TS26259 ATTRIBUTES
				if (auto stimuli = trialObject.getProperty("TS26259Attributes", "").getArray())
				{
					for (auto stimulus : *stimuli)
					{
						TS26259Attribute* con = new TS26259Attribute;

						con->name = stimulus.getProperty("name", "");

						m_testTrials[i]->addTAttribute(con);
					}
				}

				// TS26259 CONDITIONS
				if (auto stimuli = trialObject.getProperty("TS26259Conditions", "").getArray())
				{
					for (auto stimulus : *stimuli)
					{
						TS26259Condition* con = new TS26259Condition;

						File source(sceneFolder.getFullPathName() + File::getSeparatorString() + stimulus.getProperty("source", "").toString());
						File ambixconfig(ambixconfigFolder.getFullPathName() + File::getSeparatorString() + stimulus.getProperty("ambixconfig", "").toString());


						con->name = stimulus.getProperty("name", "");

						if (source.exists())
							con->filepath = source.getFullPathName();

						con->renderingOrder = stimulus.getProperty("order", "");
						con->gain = stimulus.getProperty("gain", "");
						if (ambixconfig.existsAsFile()) con->ambixConfig = ambixconfig.getFullPathName();

						m_testTrials[i]->addTCondition(con);
					}

					// randomise MUSHRA conditions
					m_testTrials[i]->randomiseMConditions();
				}

				// read rating scale for the trial
				if (auto ratings = trialObject.getProperty("ratings", "").getArray())
					m_testTrials[i]->setRatingOptions(*ratings);
			}
		}

		// randomiseTrials(); // randomisation doesn't work, needs to be verified
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
			for (int i = 0; i < testTrial->getNumberOfMConditions(); ++i)
			{
				fos << m_sessionId << ",";

				if (m_subjectData != nullptr)
				{
					fos << m_subjectData->m_id << ",";

					if (m_subjectData->m_name.isNotEmpty())
						fos << m_subjectData->m_name << ",";
					else
						fos << "-,";

					if (m_subjectData->m_age > 0)
						fos << m_subjectData->m_age << ",";
					else
						fos << "-,";
					
					if (m_subjectData->m_gender.isNotEmpty())
						fos << m_subjectData->m_gender << ",";
					else
						fos << "-,";
				}
				else
				{
					fos << "-,-,-,-,";
				}

				fos << testTrial->getId() << "," << testTrial->getMCondition(i)->name << "," << testTrial->getMCondition(i)->score << "\n";
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
	m_sessionId = Time::getCurrentTime().formatted("%y%m%d_%H%M%S");
}
