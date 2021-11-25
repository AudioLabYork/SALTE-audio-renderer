#include "AuditoryLocalisation2.h"
#include <random>

AuditoryLocalisation2::AuditoryLocalisation2()
	: m_oscTxRx(nullptr)
	, m_player(nullptr)
	, m_renderer(nullptr)
{
	formatManager.registerBasicFormats();

	m_chooseStimuliFolder.setButtonText("Select Stimuli Folder");
	m_chooseStimuliFolder.addListener(this);
	addAndMakeVisible(m_chooseStimuliFolder);

	m_testStimuliName.setEditable(false, true, false);
	m_testStimuliName.setText("*.wav", dontSendNotification);
	m_testStimuliName.setColour(Label::outlineColourId, Colours::black);
	m_testStimuliName.setJustificationType(Justification::centred);
	m_testStimuliName.onTextChange = [this] { createTestTrials(); };
	addAndMakeVisible(m_testStimuliName);

	m_testStimuliReps.setEditable(false, true, false);
	m_testStimuliReps.setText("0", dontSendNotification);
	m_testStimuliReps.setColour(Label::outlineColourId, Colours::black);
	m_testStimuliReps.setJustificationType(Justification::centred);
	m_testStimuliReps.onTextChange = [this] { createTestTrials(); };
	addAndMakeVisible(m_testStimuliReps);

	m_autoNextTrial.setToggleState(false, dontSendNotification);
	m_autoNextTrial.setButtonText("Auto mode");
	m_autoNextTrial.addListener(this);
	addAndMakeVisible(m_autoNextTrial);

	m_loopStimulus.setToggleState(false, dontSendNotification);
	m_loopStimulus.setButtonText("Loop stimulus");
	m_loopStimulus.addListener(this);
	addAndMakeVisible(m_loopStimulus);

	m_distance.setEditable(false, true, false);
	m_distance.setText("1.2", dontSendNotification);
	m_distance.setColour(Label::outlineColourId, Colours::black);
	m_distance.setJustificationType(Justification::centred);
	m_distance.onTextChange = [this] { updateRemoteInterface(); };
	addAndMakeVisible(m_distance);

	m_horizonLocked.setToggleState(false, dontSendNotification);
	m_horizonLocked.setButtonText("Locked");
	m_horizonLocked.onStateChange = [this] { updateRemoteInterface(); };
	addAndMakeVisible(m_horizonLocked);

	m_meshHorizonOn.setToggleState(true, dontSendNotification);
	m_meshHorizonOn.setButtonText("Mesh");
	m_meshHorizonOn.onStateChange = [this] { updateRemoteInterface(); };
	addAndMakeVisible(m_meshHorizonOn);

	m_pointerOn.setToggleState(false, dontSendNotification);
	m_pointerOn.setButtonText("Pointer");
	m_pointerOn.onStateChange = [this] { updateRemoteInterface(); };
	addAndMakeVisible(m_pointerOn);

	m_startStopTest.setButtonText("Start Test");
	m_startStopTest.setToggleState(false, dontSendNotification);
	m_startStopTest.addListener(this);
	addAndMakeVisible(m_startStopTest);

	m_prevTrial.setButtonText("Previous Trial");
	m_prevTrial.addListener(this);
	addAndMakeVisible(m_prevTrial);

	m_nextTrial.setButtonText("Next Trial");
	m_nextTrial.addListener(this);
	addAndMakeVisible(m_nextTrial);

	m_saveLogButton.setButtonText("Save Log");
	m_saveLogButton.addListener(this);
	m_saveLogButton.setEnabled(false);
	addAndMakeVisible(m_saveLogButton);

	m_messageCounter.setColour(Label::outlineColourId, Colours::black);
	m_messageCounter.setJustificationType(Justification::centred);
	addAndMakeVisible(m_messageCounter);

	m_lastMessage.setColour(Label::outlineColourId, Colours::black);
	m_lastMessage.setMultiLine(true, false);
	m_lastMessage.setReadOnly(true);
	m_lastMessage.setCaretVisible(false);
	m_lastMessage.setScrollbarsShown(false);
	addAndMakeVisible(m_lastMessage);

	m_logHeaderTE.setColour(Label::outlineColourId, Colours::black);
	m_logHeaderTE.setMultiLine(true, false);
	m_logHeaderTE.setReadOnly(true);
	m_logHeaderTE.setCaretVisible(false);
	m_logHeaderTE.setScrollbarsShown(false);
	m_logHeaderTE.setText(m_logHeader.replace(",", "\n"), dontSendNotification);
	addAndMakeVisible(m_logHeaderTE);

	// specify audio files dir
	setAudioFilesDir("C:/TR_FILES/local_repositories/encode-noise-ambisonics/output");

	// add config files (C:\TR_FILES\SALTE-XR-HRTF-TEST\HRTFs)
	File ambixConfigFile = File("C:/TR_FILES/local_repositories/XR-HRTF-processing/data/20211104-q2_tr/ambix_wav/xr-hrtf-5OA-50Leb.config");
	if (ambixConfigFile.existsAsFile()) ambixConfigFilesArray.add(ambixConfigFile);
	ambixConfigFile = File("C:/TR_FILES/local_repositories/XR-HRTF-processing/data/SADIE_II_selected_HRTFs/KU100/O5_3d_sn3d_50Leb_pinv_basic.config");
	if (ambixConfigFile.existsAsFile()) ambixConfigFilesArray.add(ambixConfigFile);
	ambixConfigFile = File("C:/TR_FILES/local_repositories/XR-HRTF-processing/data/SADIE_II_selected_HRTFs/KEMAR/O5_3d_sn3d_50Leb_pinv_basic.config");
	if (ambixConfigFile.existsAsFile()) ambixConfigFilesArray.add(ambixConfigFile);

	DBG("number of ambix files added: " + String(ambixConfigFilesArray.size()));

	// osc logging
	startTimerHz(60);
}

AuditoryLocalisation2::~AuditoryLocalisation2()
{
}

void AuditoryLocalisation2::init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRenderer* renderer)
{
	m_renderer = renderer;
	m_player = player;
	m_player->addChangeListener(this);
	m_oscTxRx = oscTxRx;
}

void AuditoryLocalisation2::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

	g.setColour(Colours::white);
	g.drawText(audioFilesDir.getFullPathName(), 180, 20, 440, 25, Justification::centredLeft);
	g.drawText("Number of audio files: " + String(audioFilesArray.size()), 20, 90, 440, 25, Justification::centredLeft);
	g.drawText("Number of ambix config files: " + String(ambixConfigFilesArray.size()), 20, 115, 440, 25, Justification::centredLeft);
	g.drawText("Number of trials: " + String(testTrials.size()) + ", approximate test length: " + returnHHMMSS(totalTimeOfAudioFiles * testTrials.size()), 20, 140, 440, 25, Justification::centredLeft);
	
	if(audioFilesArray.size() > 0)
		g.drawText("Current trial: " + String(m_currentTrialIndex + 1) + " of " + String(testTrials.size()), 180, 410, 300, 25, Justification::centredLeft);
}

void AuditoryLocalisation2::resized()
{
	m_chooseStimuliFolder.setBounds(20, 20, 150, 25);

	m_testStimuliName.setBounds(20, 60, 150, 25);
	m_testStimuliReps.setBounds(180, 60, 40, 25);
	
	m_autoNextTrial.setBounds(20, 180, 150, 25);
	m_loopStimulus.setBounds(20, 210, 150, 25);
	m_prevTrial.setBounds(20, 240, 100, 25);
	m_nextTrial.setBounds(140, 240, 100, 25);

	m_distance.setBounds(20, 300, 40, 25);
	m_horizonLocked.setBounds(70, 300, 70, 25);
	m_meshHorizonOn.setBounds(150, 300, 70, 25);
	m_pointerOn.setBounds(220, 300, 70, 25);

	m_logHeaderTE.setBounds(340, 180, 85, 285);
	m_lastMessage.setBounds(425, 180, 195, 285);
	m_startStopTest.setBounds(20, 410, 150, 25);
	m_saveLogButton.setBounds(20, 440, 150, 25);
	m_messageCounter.setBounds(180, 440, 150, 25);
}

void AuditoryLocalisation2::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &m_chooseStimuliFolder)
	{
		selectSrcPath();
	}
	else if (buttonThatWasClicked == &m_autoNextTrial)
	{
		m_loopStimulus.setToggleState(false, sendNotification);
		m_loopStimulus.setEnabled(!m_autoNextTrial.getToggleState());
		m_prevTrial.setEnabled(!m_autoNextTrial.getToggleState());
		m_nextTrial.setEnabled(!m_autoNextTrial.getToggleState());
	}
	else if (buttonThatWasClicked == &m_loopStimulus)
	{
		m_player->loop(m_loopStimulus.getToggleState());
	}
	else if (buttonThatWasClicked == &m_startStopTest)
	{
		if (m_startStopTest.getToggleState())
		{
			// stopping test
			m_startStopTest.setToggleState(false, dontSendNotification);
			m_startStopTest.setButtonText("Start Test");
			m_oscTxRx->removeListener(this);
			saveResults(audioFilesDir.getChildFile(m_sessionId + ".csv"));
			//m_loopStimulus.setToggleState(false, sendNotification);
		}
		else
		{
			// starting test
			oscMessageList.clear();
			createTestTrials();
			m_oscTxRx->addListener(this);
			m_activationTime = Time::getMillisecondCounterHiRes();
			m_sessionId = Time::getCurrentTime().formatted("%y%m%d_%H%M%S");
			changeTrial(0);
			m_startStopTest.setToggleState(true, dontSendNotification);
			m_startStopTest.setButtonText("Stop Test");
		}

		updateRemoteInterface();
	}
	else if (buttonThatWasClicked == &m_prevTrial)
	{
		changeTrial(m_currentTrialIndex - 1);
	}
	else if (buttonThatWasClicked == &m_nextTrial)
	{
		changeTrial(m_currentTrialIndex + 1);
	}
	else if (buttonThatWasClicked == &m_saveLogButton)
	{
		if (oscMessageList.size() > 0)
		{
			browseToSaveResults();
		}
	}
	repaint();
}

void AuditoryLocalisation2::timerCallback()
{
	m_messageCounter.setText(String(oscMessageList.size()), dontSendNotification);

	if (oscMessageList.size() > 0)
	{
		m_saveLogButton.setEnabled(true);
		
		String lastMsg = oscMessageList[oscMessageList.size() - 1];
		lastMsg = lastMsg.replace(",","\n");
		m_lastMessage.setText(lastMsg, dontSendNotification);
	}
	else
	{
		m_saveLogButton.setEnabled(false);
		m_lastMessage.setText("", dontSendNotification);
	}
}

void AuditoryLocalisation2::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == m_player)
	{
		//// send OSC
		//double time = Time::getMillisecondCounterHiRes() - m_activationTime;

		//if (audioFilesArray[m_currentTrialIndex].exists() && m_player->checkPlaybackStatus())
		//{
		//	String filename = audioFilesArray[m_currentTrialIndex].getFileName();
		//	String vis = filename.fromFirstOccurrenceOf("vis_", false, false).upToFirstOccurrenceOf("_", false, false);
		//	String azi = filename.fromFirstOccurrenceOf("azi_", false, false).upToFirstOccurrenceOf("_", false, false);
		//	String ele = filename.fromFirstOccurrenceOf("ele_", false, false).dropLastCharacters(4);
		//	time = Time::getMillisecondCounterHiRes() - m_activationTime;
		//	m_oscTxRx->sendOscMessage("/currentTrial", (int)time, (int)(m_currentTrialIndex + 1), (String)filename);
		//	time = Time::getMillisecondCounterHiRes() - m_activationTime;
		//	m_oscTxRx->sendOscMessage("/targetVisAzEl", (int)time, (int)vis.getIntValue(), (float)azi.getFloatValue(), (float)ele.getFloatValue());
		//	//sendMsgToLogWindow("vis: " + vis + ", azi: " + azi + ", ele: " + ele + ", file: " + audioFilesArray[m_currentTrialIndex].getFileName());
		//}
		//else
		//{
		//	String filename = "no stimulus present";
		//	time = Time::getMillisecondCounterHiRes() - m_activationTime;
		//	m_oscTxRx->sendOscMessage("/currentTrial", (int)time, (int)(m_currentTrialIndex + 1), (String)filename);
		//	time = Time::getMillisecondCounterHiRes() - m_activationTime;
		//	m_oscTxRx->sendOscMessage("/targetVisAzEl", (int)time, (int)0, (float)0.f, (float)0.f);
		//}

		// load next trial
		if (!m_player->checkPlaybackStatus() && m_startStopTest.getToggleState())
		{
			if (m_autoNextTrial.getToggleState()) changeTrial(m_currentTrialIndex + 1);
		}
	}
}

void AuditoryLocalisation2::changeTrial(int newTrialIndex)
{
	if (newTrialIndex != m_currentTrialIndex || newTrialIndex == 0)
	{
		if (newTrialIndex >= 0 && newTrialIndex <= testTrials.size() - 1)
		{
			m_currentTrialIndex = newTrialIndex;
			playStimulus();
		}
		else if (newTrialIndex > audioFilesArray.size() - 1)
		{
			if (m_startStopTest.getToggleState())
			{
				m_startStopTest.triggerClick();
				if(m_player->checkPlaybackStatus()) m_player->stop();
			}
		}
	}
	repaint();
}

void AuditoryLocalisation2::playStimulus()
{
	StringArray currentTrial = StringArray::fromTokens(testTrials[m_currentTrialIndex], ",", "\"");
	if (currentTrial.size() == 2)
	{
		m_player->pause();
		m_renderer->loadAmbixFile(File(currentTrial[1]));
		m_player->loadSourceToTransport(currentTrial[0]);
		m_player->play();
	}
}

void AuditoryLocalisation2::playBeep(int beep_type)
{
	m_player->pause();

	if (beep_type == 0)
		m_player->loadSourceToTransport(audioFilesDir.getFullPathName() + "/single_beep.wav");
	else if (beep_type == 1)
		m_player->loadSourceToTransport(audioFilesDir.getFullPathName() + "/double_beep.wav");

	m_player->play();
}

String AuditoryLocalisation2::getAudioFilesDir()
{
	return audioFilesDir.getFullPathName();
}

void AuditoryLocalisation2::setAudioFilesDir(String filePath)
{
	if (File(filePath).exists())
	{
		audioFilesDir = filePath;
		createTestTrials();
	}
}

bool AuditoryLocalisation2::isTestInProgress()
{
	return m_startStopTest.getToggleState();
}

void AuditoryLocalisation2::selectSrcPath()
{
	FileChooser fc("Select the stimuli folder...",
		File::getCurrentWorkingDirectory());
	
	if (fc.browseForDirectory())
	{
		audioFilesDir = fc.getResult();
		createTestTrials();
	}
}

void AuditoryLocalisation2::createTestTrials()
{
	StringArray testTrialsCopy;
	audioFilesArray.clear();
	String testName = m_testStimuliName.getText();
	int testReps = m_testStimuliReps.getText().getIntValue();

	if (testName.isNotEmpty())
	{
		DirectoryIterator iter(audioFilesDir, false, testName);
		while (iter.next())
			audioFilesArray.add(iter.getFile());
	}

	for (int i = 0; i < audioFilesArray.size(); ++i)
		for (int j = 0; j < ambixConfigFilesArray.size(); ++j)
			testTrialsCopy.add(audioFilesArray[i].getFullPathName() + "," + ambixConfigFilesArray[j].getFullPathName());

	testTrials.clear();
	for (int i = 0; i < testReps; ++i)
		testTrials.addArray(testTrialsCopy);

	std::random_device seed;
	std::mt19937 rng(seed());
	std::shuffle(testTrials.begin(), testTrials.end(), rng);

	bool dumpFileList = true;
	if (dumpFileList)
	{
		File logFile;
		logFile = audioFilesDir.getFullPathName() + "/testTrialList.csv";
		if (logFile.exists())
		{
			logFile.deleteFile();
			logFile.create();
		}
		else
		{
			logFile.create();
		}
		FileOutputStream fos(logFile);
		// create csv file header
		fos << "audiofile,ambixconfigfile\n";
		for (int i = 0; i < testTrials.size(); ++i)
			fos << testTrials[i] + "\n";
	}

	if (audioFilesArray.size() > 0 && testTrials.size() > 0)
	{
		m_player->clearPlayer();
		totalTimeOfAudioFiles = 0.0f;

		// load files to the player
		for (auto& file : audioFilesArray)
		{
			m_player->cacheFileToPlayer(file.getFullPathName());
			if (auto * reader = formatManager.createReaderFor(file))
			{
				totalTimeOfAudioFiles += reader->lengthInSamples / reader->sampleRate;
				reader->~AudioFormatReader();
			}
		}

		//m_player->cacheFileToPlayer(audioFilesDir.getFullPathName() + "/single_beep.wav");
		//m_player->cacheFileToPlayer(audioFilesDir.getFullPathName() + "/double_beep.wav");
		m_startStopTest.setEnabled(true);
	}
	else
	{
		m_startStopTest.setEnabled(false);
	}
	repaint();
}

String AuditoryLocalisation2::returnHHMMSS(double lengthInSeconds)
{
	int hours = static_cast<int>(lengthInSeconds / (60 * 60));
	int minutes = (static_cast<int>(lengthInSeconds / 60)) % 60;
	int seconds = (static_cast<int>(lengthInSeconds)) % 60;
	int millis = static_cast<int>(floor((lengthInSeconds - floor(lengthInSeconds)) * 100));

	String output = String(hours).paddedLeft('0', 2) + ":" +
		String(minutes).paddedLeft('0', 2) + ":" +
		String(seconds).paddedLeft('0', 2) + "." +
		String(millis).paddedLeft('0', 2);
	return output;
}

void AuditoryLocalisation2::updateRemoteInterface()
{
	if (m_startStopTest.getToggleState())
		m_oscTxRx->sendOscMessage("/testSceneType", (String)"test_scene_localization");
	else
		m_oscTxRx->sendOscMessage("/testSceneType", (String)"start_scene_localization");

	float distance = m_distance.getText().getFloatValue();
	bool lock = m_horizonLocked.getToggleState();
	bool mesh = m_meshHorizonOn.getToggleState();
	m_oscTxRx->sendOscMessage("/targetDistance", (float)distance, (int)lock, (int)mesh);
	bool pointer = m_pointerOn.getToggleState();
	m_oscTxRx->sendOscMessage("/pointerOn", (int)pointer);
}

void AuditoryLocalisation2::oscMessageReceived(const OSCMessage& message)
{
	processOscMessage(message);
}

void AuditoryLocalisation2::oscBundleReceived(const OSCBundle& bundle)
{
	OSCBundle::Element elem = bundle[0];
	processOscMessage(elem.getMessage());
}

void AuditoryLocalisation2::processOscMessage(const OSCMessage& message)
{
	if (message.getAddressPattern().toString() == "/attenuation")
	{
		m_player->setAttenuation(message[0].getFloat32());
	}

	if (message.getAddressPattern().toString() == "/pointing")
	{
		// add message to the message list
		String arguments;
		for (int i = 0; i < message.size(); ++i)
		{
			if (message[i].isString()) arguments += "," + message[i].getString();
			else if (message[i].isFloat32()) arguments += "," + String(message[i].getFloat32());
			else if (message[i].isInt32()) arguments += "," + String(message[i].getInt32());
		}

		double time = Time::getMillisecondCounterHiRes() - m_activationTime;
		String messageText = String((int)time) + "," + String(m_currentTrialIndex + 1) + ",";
		messageText += testTrials[m_currentTrialIndex] + ","; // audio filename and ambix config name
		messageText += message.getAddressPattern().toString() + arguments + "\n";

		oscMessageList.add(messageText);
	}

	if (message.getAddressPattern().toString() == "/next_trial")
	{
		// control the test component
		if (message[0].isString() && message[0].getString() == "confirm")
		{
			m_nextTrial.triggerClick();
		}
	}

}

void AuditoryLocalisation2::browseToSaveResults()
{
	File browseLocation = File::getCurrentWorkingDirectory();
	if (audioFilesDir.exists())
		browseLocation = audioFilesDir;

	FileChooser fc("Select or create results export file...",
		browseLocation,
		"*.csv",
		true);

	if (fc.browseForFileToSave(true))
	{
		saveResults(fc.getResult());
	}
}

void AuditoryLocalisation2::saveResults(File results)
{
	if (!results.exists())
		results.create();

	FileOutputStream fos(results);

	// create csv file header
	fos << m_logHeader;
	for (int i = 0; i < oscMessageList.size(); ++i)
		fos << oscMessageList[i];
}

void AuditoryLocalisation2::sendMsgToLogWindow(String message)
{
	currentMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
}
