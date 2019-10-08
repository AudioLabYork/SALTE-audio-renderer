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
	std::random_device seed;
	std::mt19937 rng(seed());
	std::shuffle(m_testTrials.begin(), m_testTrials.end(), rng);
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

		// get the array of trials
		auto trials = json.getProperty("trials", "").getArray();

		// itterate through each trial
		for (auto trial : *trials)
		{
			// create a new trial
			// this will be added to an array at the end when all configuration is complete
			TestTrial* testTrial = new TestTrial;

			testTrial->init(trial.getProperty("id", ""));
			testTrial->setTrialName(trial.getProperty("name", ""));
			testTrial->setTrialInstruction(trial.getProperty("instruction", ""));

			File sceneFolder(sessionFile.getParentDirectory().getFullPathName() + "/" + trial.getProperty("scenefolder", "").toString());
			File ambixconfigFolder(sessionFile.getParentDirectory().getFullPathName() + "/" + trial.getProperty("ambixconfigfolder", "").toString());

			if (sceneFolder.exists())
			{
				// MUSHRA REFERENCE
				if (auto referenceStimuli = trial.getProperty("MushraReference", "").getArray())
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

							if(ambixconfig.existsAsFile())
								ref->ambixConfig = ambixconfig.getFullPathName();

							testTrial->addMReference(ref);
						}
					}
				}

				// MUSHRA CONDITIONS
				if (auto stimuli = trial.getProperty("MushraConditions", "").getArray())
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

						testTrial->addMCondition(con);
					}

					// randomise MUSHRA conditions
					testTrial->randomiseMConditions();
				}

				// TS26259 ATTRIBUTES
				if (auto stimuli = trial.getProperty("TS26259Attributes", "").getArray())
				{
					for (auto stimulus : *stimuli)
					{
						TS26259Attribute* con = new TS26259Attribute;

						con->name = stimulus.getProperty("name", "");

						testTrial->addTAttribute(con);
					}
				}

				// TS26259 CONDITIONS
				if (auto stimuli = trial.getProperty("TS26259Conditions", "").getArray())
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
						
						if (ambixconfig.existsAsFile())
							con->ambixConfig = ambixconfig.getFullPathName();

						testTrial->addTCondition(con);
					}

					// randomise MUSHRA conditions
					testTrial->randomiseMConditions();
				}

				// read rating scale for the trial
				if (auto ratings = trial.getProperty("ratings", "").getArray())
					testTrial->setRatingOptions(*ratings);
			}

			// add this test trial to the array
			m_testTrials.add(testTrial);
		}

		// randomiseTrials(); // randomisation switched off for the presentation purpose
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
