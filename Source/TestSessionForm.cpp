#include "TestSessionForm.h"

TestSessionForm::TestSessionForm()
	: m_session(nullptr)
	, m_shouldSaveSessionSettings(false)
{
	m_labelSession.setText("Session Data", NotificationType::dontSendNotification);
	addAndMakeVisible(m_labelSession);

	m_labelSessionName.setText("Session Name:", NotificationType::dontSendNotification);
	m_labelSessionName.setJustificationType(Justification::centredRight);
	addAndMakeVisible(m_labelSessionName);
	m_labelSessionFile.setText("Session File:", NotificationType::dontSendNotification);
	m_labelSessionFile.setJustificationType(Justification::centredRight);
	addAndMakeVisible(m_labelSessionFile);
	m_labelExportFile.setText("Export File:", NotificationType::dontSendNotification);
	m_labelExportFile.setJustificationType(Justification::centredRight);
	addAndMakeVisible(m_labelExportFile);

	addAndMakeVisible(m_editSessionName);
	m_btnSessionFile.setButtonText("Select file...");
	m_btnSessionFile.addListener(this);
	addAndMakeVisible(m_btnSessionFile);
	m_btnExportFile.setButtonText("Select file...");
	m_btnExportFile.addListener(this);
	addAndMakeVisible(m_btnExportFile);

	m_btnStickySession.setButtonText("Save session settings");
	m_btnStickySession.addListener(this);
	addAndMakeVisible(m_btnStickySession);

	m_labelSubject.setText("Subject Data", NotificationType::dontSendNotification);
	addAndMakeVisible(m_labelSubject);

	m_labelName.setText("Name:", NotificationType::dontSendNotification);
	m_labelName.setJustificationType(Justification::centredRight);
	addAndMakeVisible(m_labelName);
	m_labelAge.setText("Age:", NotificationType::dontSendNotification);
	m_labelAge.setJustificationType(Justification::centredRight);
	addAndMakeVisible(m_labelAge);
	m_labelGender.setText("Gender:", NotificationType::dontSendNotification);
	m_labelGender.setJustificationType(Justification::centredRight);
	addAndMakeVisible(m_labelGender);

	addAndMakeVisible(m_editName);
	addAndMakeVisible(m_editAge);
	addAndMakeVisible(m_editGender);
	
	m_btnAnon.setButtonText("Anonymize subject");
	m_btnAnon.addListener(this);
	addAndMakeVisible(m_btnAnon);
	
	m_btnAgree.setButtonText("Consent Form Signed");
	addAndMakeVisible(m_btnAgree);

	m_btnBegin.setButtonText("Begin");
	m_btnBegin.addListener(this);
	addAndMakeVisible(m_btnBegin);

	startTimer(100);
}

TestSessionForm::~TestSessionForm()
{
	stopTimer();
}

void TestSessionForm::init(TestSession* session)
{
	if ((session == nullptr))
	{
		jassertfalse;
		return;
	}
	
	m_session = session;
}

void TestSessionForm::reset()
{
	m_session->reset();
	
	if (!m_shouldSaveSessionSettings)
	{
		m_editSessionName.setText("");
		m_btnSessionFile.setButtonText("Select file...");
		m_btnExportFile.setButtonText("Select file...");
	}

	m_editName.setText("");
	m_editAge.setText("");
	m_editGender.setText("");

	m_btnAgree.setToggleState(false, NotificationType::dontSendNotification);
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
	m_labelSession.setBounds(20, 20, 100, 30);

	m_labelSessionName.setBounds(30, 55, 120, 30);
	m_labelSessionFile.setBounds(30, 90, 120, 30);
	m_labelExportFile.setBounds(30, 125, 120, 30);

	m_editSessionName.setBounds(155, 55, 250, 30);
	m_btnSessionFile.setBounds(155, 90, 250, 30);
	m_btnExportFile.setBounds(155, 125, 250, 30);

	m_btnStickySession.setBounds(410, 125, 200, 30);

	m_labelSubject.setBounds(20, 155, 100, 30);

	m_labelName.setBounds(30, 190, 120, 30);
	m_labelAge.setBounds(30, 225, 120, 30);
	m_labelGender.setBounds(30, 260, 120, 30);

	m_editName.setBounds(155, 190, 250, 30);
	m_editAge.setBounds(155, 225, 250, 30);
	m_editGender.setBounds(155, 260, 250, 30);
	
	m_btnAnon.setBounds(410, 260, 200, 30);

	m_btnAgree.setBounds(155, 295, 300, 30);
	m_btnBegin.setBounds(305, 330, 100, 30);
}

void TestSessionForm::timerCallback()
{
	if (m_editSessionName.getText().isEmpty() || m_sessionFile.getFullPathName().isEmpty() || m_exportFile.getFullPathName().isEmpty() ||
		((m_editName.getText().isEmpty() || m_editAge.getText().isEmpty() || m_editGender.getText().isEmpty()) && !m_btnAnon.getToggleState()) ||
		(!m_btnAgree.getToggleState())
		)
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
	if (button == &m_btnBegin)
	{
		m_session->loadSession(m_sessionFile);
		m_session->setExportFile(m_exportFile);

		std::unique_ptr<SubjectData> subject = std::make_unique<SubjectData>();
		
		// only collect subject data if it is not anonymous
		
		subject->m_id = Time::getCurrentTime().formatted("%y%m%d_%H%M%S");

		if (!m_btnAnon.getToggleState())
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
			m_btnSessionFile.setButtonText(m_sessionFile.getFullPathName());
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
				fos << "ses_id,sub_id,sub_name,sub_age,sub_gen,trial_id,con_name,con_score\n";
			}

			m_btnExportFile.setButtonText(m_exportFile.getFullPathName());
		}
#endif
	}
	else if (button == &m_btnStickySession)
	{
		m_shouldSaveSessionSettings = m_btnStickySession.getToggleState();
	}
}
