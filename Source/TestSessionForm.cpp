#include "TestSessionForm.h"

TestSessionForm::TestSessionForm()
	: m_session(nullptr)
	, m_shouldSaveSessionSettings(true)
	, m_anonymizeSubject(true)
{
	m_labelSession.setText("Session Data", NotificationType::dontSendNotification);
	addAndMakeVisible(m_labelSession);

	m_labelSessionFile.setText("Configuration File:", NotificationType::dontSendNotification);
	m_labelSessionFile.setJustificationType(Justification::centredLeft);
	addAndMakeVisible(m_labelSessionFile);

	m_labelExportFile.setText("Results File:", NotificationType::dontSendNotification);
	m_labelExportFile.setJustificationType(Justification::centredLeft);
	addAndMakeVisible(m_labelExportFile);

	m_btnSessionFile.setButtonText("Select file...");
	m_btnSessionFile.addListener(this);
	addAndMakeVisible(m_btnSessionFile);
	m_btnExportFile.setButtonText("Select file...");
	m_btnExportFile.addListener(this);
	addAndMakeVisible(m_btnExportFile);

	m_createRndSubjectIDButton.setButtonText("Random Subject ID");
	m_createRndSubjectIDButton.addListener(this);
	addAndMakeVisible(m_createRndSubjectIDButton);

	m_labelSubject.setText("Subject Data", NotificationType::dontSendNotification);
	addAndMakeVisible(m_labelSubject);
	m_labelSubjectID.setText("Subject ID:", NotificationType::dontSendNotification);
	m_labelSubjectID.setJustificationType(Justification::centredRight);
	addAndMakeVisible(m_labelSubjectID);
	addAndMakeVisible(m_editSubjectID);

	m_labelName.setText("Name:", NotificationType::dontSendNotification);
	m_labelName.setJustificationType(Justification::centredRight);
	m_labelAge.setText("Age:", NotificationType::dontSendNotification);
	m_labelAge.setJustificationType(Justification::centredRight);
	m_labelGender.setText("Gender:", NotificationType::dontSendNotification);
	m_labelGender.setJustificationType(Justification::centredRight);

	// load settings
	initSettings();
	if (TestSessionFormSettings.getUserSettings()->getBoolValue("loadSettingsFile"))
	{
		loadSettings();
	}

	// show name / age / gender fields
	if (!m_anonymizeSubject)
	{

		addAndMakeVisible(m_labelName);
		addAndMakeVisible(m_labelAge);
		addAndMakeVisible(m_labelGender);
		addAndMakeVisible(m_editName);
		addAndMakeVisible(m_editAge);
		addAndMakeVisible(m_editGender);
	}
	
	m_btnAnon.setButtonText("Anonymize subject");
	m_btnAnon.setToggleState(m_anonymizeSubject, NotificationType::dontSendNotification);
	m_btnAnon.addListener(this);
	addAndMakeVisible(m_btnAnon);
	
	m_btnAgree.setButtonText("Consent Form Signed");
	m_btnAgree.setToggleState(true, NotificationType::dontSendNotification);
	addAndMakeVisible(m_btnAgree);

	m_btnBegin.setButtonText("Begin");
	m_btnBegin.addListener(this);
	addAndMakeVisible(m_btnBegin);

	startTimer(100);
}

TestSessionForm::~TestSessionForm()
{
	stopTimer();
	saveSettings();
}

void TestSessionForm::init(TestSession* session)
{
	if ((session == nullptr))
	{
		jassertfalse;
		return;
	}
	
	m_session = session;

	createRandomSubjectID();
}

void TestSessionForm::reset()
{
	m_session->reset();

	//m_editName.setText("");
	//m_editAge.setText("");
	//m_editGender.setText("");

	//m_btnAgree.setToggleState(false, NotificationType::dontSendNotification);
}

void TestSessionForm::addListener(Listener* newListener)
{
	testSessionListeners.add(newListener);
}

void TestSessionForm::removeListener(Listener* listener)
{
	testSessionListeners.remove(listener);
}

void TestSessionForm::paint(Graphics& g)
{
}

void TestSessionForm::resized()
{

	m_labelSession.setBounds(20, 20, 100, 25);

	m_labelSessionFile.setBounds(30, 50, 500, 25);
	m_labelExportFile.setBounds(30, 80, 500, 25);
	m_btnSessionFile.setBounds(530, 50, 80, 25);
	m_btnExportFile.setBounds(530, 80, 80, 25);

	m_labelSubject.setBounds(20, 120, 100, 25);

	m_labelSubjectID.setBounds(30, 150, 120, 25);
	m_labelName.setBounds(30, 180, 120, 25);
	m_labelAge.setBounds(30, 210, 120, 25);
	m_labelGender.setBounds(30, 240, 120, 25);

	m_editSubjectID.setBounds(155, 150, 250, 25);
	m_editName.setBounds(155, 180, 250, 25);
	m_editAge.setBounds(155, 210, 250, 25);
	m_editGender.setBounds(155, 240, 250, 25);
	
	m_createRndSubjectIDButton.setBounds(410, 150, 200, 25);
	m_btnAnon.setBounds(410, 180, 200, 25);

	m_btnAgree.setBounds(155, 295, 300, 25);
	m_btnBegin.setBounds(305, 330, 100, 25);
}

void TestSessionForm::timerCallback()
{
	if (m_sessionFile.getFullPathName().isEmpty() ||
		m_exportFile.getFullPathName().isEmpty() ||
		(!m_anonymizeSubject && (m_editSubjectID.getText().isEmpty() || m_editName.getText().isEmpty() || m_editAge.getText().isEmpty() || m_editGender.getText().isEmpty())) ||
		(m_anonymizeSubject && m_editSubjectID.getText().isEmpty()) ||
		!m_btnAgree.getToggleState())
	{
		m_btnBegin.setEnabled(false);
	}
	else
	{
		m_btnBegin.setEnabled(true);
	}
}

void TestSessionForm::buttonClicked(Button* button)
{
	if (button == &m_btnAnon)
	{
		m_anonymizeSubject = m_btnAnon.getToggleState();
		if (m_anonymizeSubject)
		{
			m_labelName.setVisible(false);
			m_labelAge.setVisible(false);
			m_labelGender.setVisible(false);
			m_editName.setVisible(false);
			m_editAge.setVisible(false);
			m_editGender.setVisible(false);
		}
		else
		{
			addAndMakeVisible(m_labelName);
			addAndMakeVisible(m_labelAge);
			addAndMakeVisible(m_labelGender);
			addAndMakeVisible(m_editName);
			addAndMakeVisible(m_editAge);
			addAndMakeVisible(m_editGender);
		}

	}
	else if (button == &m_createRndSubjectIDButton)
	{
		createRandomSubjectID();
	}
	else if (button == &m_btnBegin)
	{
		m_session->loadSession(m_sessionFile);
		m_session->setExportFile(m_exportFile);

		std::unique_ptr<SubjectData> subject = std::make_unique<SubjectData>();
		
		
		// get subject ID
		subject->m_id = m_editSubjectID.getText();;

		// only collect subject data (name, age, gender) if it is not anonymous
		if (!m_anonymizeSubject)
		{
			subject->m_name = m_editName.getText();
			subject->m_age = m_editAge.getText().getIntValue();
			subject->m_gender = m_editGender.getText();
		}

		m_session->setSubjectData(std::move(subject));
		m_session->begin();

		setVisible(false);
		testSessionListeners.call([this](Listener& l) { l.formCompleted(); });
	}
	else if (button == &m_btnSessionFile)
	{
#if JUCE_MODAL_LOOPS_PERMITTED
		FileChooser fc("Select a Test Session file to open...",
			File::getCurrentWorkingDirectory(),
			"*.json",
			true);

		if (fc.browseForFileToOpen())
		{
			m_sessionFile = fc.getResult();
			m_labelSessionFile.setText("Configuration File: " + m_sessionFile.getFullPathName(), NotificationType::dontSendNotification);
		}
#endif
	}
	else if (button == &m_btnExportFile)
	{
#if JUCE_MODAL_LOOPS_PERMITTED
		FileChooser fc("Select or create results export file...",
			File::getCurrentWorkingDirectory(),
			"*.csv",
			true);

		if (fc.browseForFileToSave(true))
		{
			m_exportFile = fc.getResult();

			if (!m_exportFile.exists())
			{
				m_exportFile.create();

				FileOutputStream fos(m_exportFile);
				fos << "ses_date,sub_id,sub_name,sub_age,sub_gen,trial_id,con_name,con_score\n";
			}

			m_labelExportFile.setText("Results File: " + m_exportFile.getFullPathName(), NotificationType::dontSendNotification);
		}
#endif
	}
}

void TestSessionForm::createRandomSubjectID()
{
	auto randomInt = Random::getSystemRandom().nextInt();
	m_editSubjectID.setText(String(randomInt).getLastCharacters(6));
}

void TestSessionForm::initSettings()
{
	PropertiesFile::Options options;
	// options.applicationName = ProjectInfo::projectName;
	options.applicationName = "SALTETestSessionFormSettings";
	options.filenameSuffix = ".conf";
	options.osxLibrarySubFolder = "Application Support";
	options.folderName = File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile).getParentDirectory().getFullPathName();
	options.storageFormat = PropertiesFile::storeAsXML;
	//    PropertiesFile::reload();
	TestSessionFormSettings.setStorageParameters(options);
}

void TestSessionForm::loadSettings()
{
	m_sessionFile = TestSessionFormSettings.getUserSettings()->getValue("configFilePath");
	m_exportFile = TestSessionFormSettings.getUserSettings()->getValue("resultsFilePath");
	m_labelSessionFile.setText("Configuration File: " + m_sessionFile.getFullPathName(), NotificationType::dontSendNotification);
	m_labelExportFile.setText("Results File: " + m_exportFile.getFullPathName(), NotificationType::dontSendNotification);
}

void TestSessionForm::saveSettings()
{
	TestSessionFormSettings.getUserSettings()->setValue("configFilePath", m_sessionFile.getFullPathName());
	TestSessionFormSettings.getUserSettings()->setValue("resultsFilePath", m_exportFile.getFullPathName());

	TestSessionFormSettings.getUserSettings()->setValue("loadSettingsFile", true);
}
