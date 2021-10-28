#include "AuditoryLocalisation.h"
#include <random>

AuditoryLocalisation::AuditoryLocalisation()
	: m_oscTxRx(nullptr)
	, m_player(nullptr)
	, m_renderer(nullptr)
{
	formatManager.registerBasicFormats();

	m_chooseStimuliFolder.setButtonText("Select Stimuli Folder");
	m_chooseStimuliFolder.addListener(this);
	addAndMakeVisible(m_chooseStimuliFolder);

	m_calStimuliName.setEditable(false, true, false);
	m_calStimuliName.setText("stim_vis_1_*.wav", dontSendNotification);
	m_calStimuliName.setColour(Label::outlineColourId, Colours::black);
	m_calStimuliName.setJustificationType(Justification::centred);
	m_calStimuliName.onTextChange = [this] { indexAudioFiles(); };
	addAndMakeVisible(m_calStimuliName);

	m_testStimuliName.setEditable(false, true, false);
	m_testStimuliName.setText("stim_vis_0_*.wav", dontSendNotification);
	m_testStimuliName.setColour(Label::outlineColourId, Colours::black);
	m_testStimuliName.setJustificationType(Justification::centred);
	m_testStimuliName.onTextChange = [this] { indexAudioFiles(); };
	addAndMakeVisible(m_testStimuliName);

	m_calStimuliReps.setEditable(false, true, false);
	m_calStimuliReps.setText("0", dontSendNotification);
	m_calStimuliReps.setColour(Label::outlineColourId, Colours::black);
	m_calStimuliReps.setJustificationType(Justification::centred);
	m_calStimuliReps.onTextChange = [this] { indexAudioFiles(); };
	addAndMakeVisible(m_calStimuliReps);

	m_testStimuliReps.setEditable(false, true, false);
	m_testStimuliReps.setText("0", dontSendNotification);
	m_testStimuliReps.setColour(Label::outlineColourId, Colours::black);
	m_testStimuliReps.setJustificationType(Justification::centred);
	m_testStimuliReps.onTextChange = [this] { indexAudioFiles(); };
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
	m_distance.onTextChange = [this] { setHorizon(); };
	addAndMakeVisible(m_distance);

	m_horizonLocked.setToggleState(false, dontSendNotification);
	m_horizonLocked.setButtonText("Locked");
	m_horizonLocked.onStateChange = [this] { setHorizon(); };
	addAndMakeVisible(m_horizonLocked);

	m_meshHorizonOn.setToggleState(true, dontSendNotification);
	m_meshHorizonOn.setButtonText("Mesh");
	m_meshHorizonOn.onStateChange = [this] { setHorizon(); };
	addAndMakeVisible(m_meshHorizonOn);

	m_pointerOn.setToggleState(false, dontSendNotification);
	m_pointerOn.setButtonText("Pointer");
	m_pointerOn.onStateChange = [this] { setHorizon(); };
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

	// osc logging
	startTimerHz(60);

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
}

AuditoryLocalisation::~AuditoryLocalisation()
{
}

void AuditoryLocalisation::init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRenderer* renderer)
{
	m_renderer = renderer;
	m_player = player;
	m_player->addChangeListener(this);
	m_oscTxRx = oscTxRx;
}

void AuditoryLocalisation::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

	g.setColour(Colours::white);
	g.drawText(audioFilesDir.getFullPathName(), 180, 20, 440, 25, Justification::centredLeft);
	g.drawText("Number of trials: " + String(audioFilesArray.size()) + ", total length: " + returnHHMMSS(totalTimeOfAudioFiles), 20, 120, 440, 25, Justification::centredLeft);
	
	if(audioFilesArray.size() > 0)
		g.drawText("Current trial: " + String(m_currentTrialIndex + 1) + " of " + String(audioFilesArray.size()), 180, 410, 300, 25, Justification::centredLeft);

	// g.setFont(20.0f);
	// g.drawText("The localisation component is under development.", getLocalBounds(), Justification::centred);
}

void AuditoryLocalisation::resized()
{
	m_chooseStimuliFolder.setBounds(20, 20, 150, 25);

	m_calStimuliName.setBounds(20, 60, 150, 25);
	m_testStimuliName.setBounds(20, 90, 150, 25);
	m_calStimuliReps.setBounds(180, 60, 40, 25);
	m_testStimuliReps.setBounds(180, 90, 40, 25);
	
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

void AuditoryLocalisation::buttonClicked(Button* buttonThatWasClicked)
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
			//m_player->stop();
			m_startStopTest.setToggleState(false, dontSendNotification);
			m_startStopTest.setButtonText("Start Test");
			m_oscTxRx->removeListener(this);
			m_loopStimulus.setToggleState(false, sendNotification);
		}
		else
		{
			oscMessageList.clear();
			setHorizon();
			indexAudioFiles();
			m_oscTxRx->addListener(this);
			m_activationTime = Time::getMillisecondCounterHiRes();
			changeTrial(0);
			m_startStopTest.setToggleState(true, dontSendNotification);
			m_startStopTest.setButtonText("Stop Test");
		}
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
			saveLog();
		}
	}
	repaint();
}

void AuditoryLocalisation::timerCallback()
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

void AuditoryLocalisation::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == m_player)
	{
		if (!m_player->checkPlaybackStatus() && m_startStopTest.getToggleState())
		{
			if (m_autoNextTrial.getToggleState()) changeTrial(m_currentTrialIndex + 1);
		}
	}
}

void AuditoryLocalisation::changeTrial(int newTrialIndex)
{
	if (newTrialIndex != m_currentTrialIndex || newTrialIndex == 0)
	{
		if (newTrialIndex >= 0 && newTrialIndex <= audioFilesArray.size() - 1)
		{
			m_currentTrialIndex = newTrialIndex;
			playStimulus();
		}
		else if (newTrialIndex > audioFilesArray.size() - 1)
		{
			if (m_startStopTest.getToggleState())
			{
				m_startStopTest.triggerClick();
			}
		}
	}
	repaint();
}

void AuditoryLocalisation::playStimulus()
{
	String filename = audioFilesArray[m_currentTrialIndex].getFileName();
	String vis = filename.fromFirstOccurrenceOf("vis_", false, false).upToFirstOccurrenceOf("_", false, false);
	String azi = filename.fromFirstOccurrenceOf("azi_", false, false).upToFirstOccurrenceOf("_", false, false);
	String ele = filename.fromFirstOccurrenceOf("ele_", false, false).dropLastCharacters(4);
	sendMsgToLogWindow("vis: " + vis + ", azi: " + azi + ", ele: " + ele + ", file: " + audioFilesArray[m_currentTrialIndex].getFileName());
	m_oscTxRx->sendOscMessage("/targetVisAzEl", (int)vis.getIntValue(), (float)azi.getFloatValue(), (float)ele.getFloatValue());

	m_player->pause();
	m_player->loadSourceToTransport(audioFilesArray[m_currentTrialIndex].getFullPathName());
	m_player->play();
}

void AuditoryLocalisation::playBeep(int beep_type)
{
	m_player->pause();

	if (beep_type == 0)
		m_player->loadSourceToTransport(audioFilesDir.getFullPathName() + "/single_beep.wav");
	else if (beep_type == 1)
		m_player->loadSourceToTransport(audioFilesDir.getFullPathName() + "/double_beep.wav");

	m_player->play();
}

String AuditoryLocalisation::getAudioFilesDir()
{
	return audioFilesDir.getFullPathName();
}

void AuditoryLocalisation::setAudioFilesDir(String filePath)
{
	if (File(filePath).exists())
	{
		audioFilesDir = filePath;
		indexAudioFiles();
	}
}

void AuditoryLocalisation::selectSrcPath()
{
	FileChooser fc("Select the stimuli folder...",
		File::getCurrentWorkingDirectory());
	
	if (fc.browseForDirectory())
	{
		audioFilesDir = fc.getResult();
		indexAudioFiles();
	}
}

void AuditoryLocalisation::indexAudioFiles()
{
	audioFilesArray.clear();
	Array<File> calFilesInDir, audioFilesInDir;
	String calName = m_calStimuliName.getText();
	int calReps = m_calStimuliReps.getText().getIntValue();
	String testName = m_testStimuliName.getText();
	int testReps = m_testStimuliReps.getText().getIntValue();

	if (testName.isNotEmpty() && calName.isNotEmpty() && testReps > 0)
	{
		calFilesInDir.clear();
		DirectoryIterator iter1(audioFilesDir, false, calName);
		while (iter1.next())
			calFilesInDir.add(iter1.getFile());

		audioFilesInDir.clear();
		DirectoryIterator iter2(audioFilesDir, false, testName);
		while (iter2.next())
			audioFilesInDir.add(iter2.getFile());

		for (int i = 0; i < testReps; ++i)
		{
			std::random_device seed;
			std::mt19937 rng(seed());

			std::shuffle(calFilesInDir.begin(), calFilesInDir.end(), rng);
			audioFilesArray.addArray(calFilesInDir);

			std::shuffle(audioFilesInDir.begin(), audioFilesInDir.end(), rng);
			audioFilesArray.addArray(audioFilesInDir);
		}

		std::random_device seed;
		std::mt19937 rng(seed());

		std::shuffle(calFilesInDir.begin(), calFilesInDir.end(), rng);
		audioFilesArray.addArray(calFilesInDir);
	}

	/*
	audioFilesArray.clear();
	Array<File> audioFilesInDir;
	
	// create the test file array (visual stimuli)
	audioFilesInDir.clear();
	String calName = m_calStimuliName.getText();
	int calReps = m_calStimuliReps.getText().getIntValue();

	if (calName.isNotEmpty() && calReps > 0)
	{
		DirectoryIterator iter1(audioFilesDir, false, calName);
		while (iter1.next())
			audioFilesInDir.add(iter1.getFile());

		for (int i = 0; i < calReps; ++i)
		{
			std::random_device seed;
			std::mt19937 rng(seed());
			std::shuffle(audioFilesInDir.begin(), audioFilesInDir.end(), rng);
			audioFilesArray.addArray(audioFilesInDir);
		}
	}

	// create the test file array (audio stimuli)
	audioFilesInDir.clear();
	String testName = m_testStimuliName.getText();
	int testReps = m_testStimuliReps.getText().getIntValue();

	if (testName.isNotEmpty() && testReps > 0)
	{
		DirectoryIterator iter2(audioFilesDir, false, testName);
		while (iter2.next())
			audioFilesInDir.add(iter2.getFile());

		for (int i = 0; i < testReps; ++i)
		{
			std::random_device seed;
			std::mt19937 rng(seed());
			std::shuffle(audioFilesInDir.begin(), audioFilesInDir.end(), rng);
			audioFilesArray.addArray(audioFilesInDir);
		}
	}

	// repeat cal stimuli
	audioFilesInDir.clear();

	if (calName.isNotEmpty() && calReps > 0)
	{
		DirectoryIterator iter1(audioFilesDir, false, calName);
		while (iter1.next())
			audioFilesInDir.add(iter1.getFile());

		for (int i = 0; i < calReps; ++i)
		{
			std::random_device seed;
			std::mt19937 rng(seed());
			std::shuffle(audioFilesInDir.begin(), audioFilesInDir.end(), rng);
			audioFilesArray.addArray(audioFilesInDir);
		}
	}
	*/

	/* 
	bool dumpFileList = true;
	if (dumpFileList)
	{
		File logFile;
		logFile = audioFilesDir.getFullPathName() + "/filelist.csv";
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
		fos << "filename\n";
		for (int i = 0; i < audioFilesArray.size(); ++i)
			fos << audioFilesArray[i].getFileName() + "\n";
	}
	*/

	m_player->clearPlayer();
	totalTimeOfAudioFiles = 0.0f;

	if (audioFilesArray.size() > 0)
	{
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

		m_player->cacheFileToPlayer(audioFilesDir.getFullPathName() + "/single_beep.wav");
		m_player->cacheFileToPlayer(audioFilesDir.getFullPathName() + "/double_beep.wav");
		m_startStopTest.setEnabled(true);
	}
	else
	{
		m_startStopTest.setEnabled(false);
	}
	repaint();
}

String AuditoryLocalisation::returnHHMMSS(double lengthInSeconds)
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

void AuditoryLocalisation::setHorizon()
{
	float distance = m_distance.getText().getFloatValue();
	bool lock = m_horizonLocked.getToggleState();
	bool mesh = m_meshHorizonOn.getToggleState();
	m_oscTxRx->sendOscMessage("/targetDistance", (float)distance, (int)lock, (int)mesh);
	bool pointer = m_pointerOn.getToggleState();
	m_oscTxRx->sendOscMessage("/pointerOn", (int)pointer);
}

void AuditoryLocalisation::oscMessageReceived(const OSCMessage& message)
{
	processOscMessage(message);
}

void AuditoryLocalisation::oscBundleReceived(const OSCBundle& bundle)
{
	OSCBundle::Element elem = bundle[0];
	processOscMessage(elem.getMessage());
}

void AuditoryLocalisation::processOscMessage(const OSCMessage& message)
{
	if (message.getAddressPattern().toString() == "/etptr")
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

		if (audioFilesArray[m_currentTrialIndex].exists() && m_player->checkPlaybackStatus())
		{
			messageText += audioFilesArray[m_currentTrialIndex].getFileName() + "," + message.getAddressPattern().toString() + arguments + "\n";
		}
		else
		{
			messageText += "no stimulus present," + message.getAddressPattern().toString() + arguments + "\n";
		}

		oscMessageList.add(messageText);
	}

	if (message.getAddressPattern().toString() == "/control")
	{
		// control the test component
		if (message[0].isString() && message[0].getString() == "confirm")
		{
			m_nextTrial.triggerClick(); //changeTrial(m_currentTrialIndex + 1);
		}
	}

}

void AuditoryLocalisation::saveLog()
{
	FileChooser fc("Select or create results export file...",
		File::getCurrentWorkingDirectory(),
		"*.csv",
		true);

	if (fc.browseForFileToSave(true))
	{
		File logFile;

		logFile = fc.getResult();

		if (!logFile.exists())
			logFile.create();

		FileOutputStream fos(logFile);

		// create csv file header
		fos << m_logHeader;
		for (int i = 0; i < oscMessageList.size(); ++i)
			fos << oscMessageList[i];
	}
}

void AuditoryLocalisation::sendMsgToLogWindow(String message)
{
	currentMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
}
