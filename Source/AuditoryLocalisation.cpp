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

	m_startTest.setButtonText("Start Test");
	m_startTest.setToggleState(false, dontSendNotification);
	m_startTest.addListener(this);
	addAndMakeVisible(m_startTest);

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

	addAndMakeVisible(messageCounter);
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
	g.drawText("Number of trials: " + String(audioFilesArray.size()) + ", total length: " + returnHHMMSS(totalTimeOfAudioFiles), 180, 50, 440, 25, Justification::centredLeft);
	
	if(audioFilesArray.size() > 0)
		g.drawText("Current trial: " + String(currentTrialIndex + 1) + " of " + String(audioFilesArray.size()), 180, 80, 440, 25, Justification::centredLeft);

	g.setFont(20.0f);
	g.drawText("The localisation component is under development.", getLocalBounds(), Justification::centred);
}

void AuditoryLocalisation::resized()
{
	m_chooseStimuliFolder.setBounds(20, 20, 150, 25);
	m_startTest.setBounds(20, 50, 150, 25);
	m_prevTrial.setBounds(20, 420, 100, 25);
	m_nextTrial.setBounds(140, 420, 100, 25);

	m_saveLogButton.setBounds(20, 110, 150, 25);
	messageCounter.setBounds(20, 140, 150, 25);
}

void AuditoryLocalisation::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &m_chooseStimuliFolder)
	{
		selectSrcPath();
	}
	else if (buttonThatWasClicked == &m_startTest)
	{
		if (audioFilesArray.isEmpty())
		{
			indexAudioFiles();
		}

		if (m_startTest.getToggleState())
		{
			currentTrialIndex = 0;
			m_startTest.setToggleState(false, dontSendNotification);
			m_startTest.setButtonText("Start Test");
			m_oscTxRx->removeListener(this);
		}
		else
		{
			oscMessageList.clear();
			m_oscTxRx->addListener(this);
			activationTime = Time::getMillisecondCounterHiRes();

			loadFile();
			m_startTest.setToggleState(true, dontSendNotification);
			m_startTest.setButtonText("Stop Test");
		}
	}
	else if (buttonThatWasClicked == &m_prevTrial)
	{
		if(currentTrialIndex > 0)
		{
			currentTrialIndex--;
			loadFile();
		}
	}
	else if (buttonThatWasClicked == &m_nextTrial)
	{
		if (currentTrialIndex < audioFilesArray.size() - 1)
		{
			currentTrialIndex++;
			loadFile();
		}
		else
		{
			m_startTest.triggerClick();
		}
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
	messageCounter.setText(String(oscMessageList.size()), dontSendNotification);

	if(oscMessageList.size() > 0)
		m_saveLogButton.setEnabled(true);
	else
		m_saveLogButton.setEnabled(false);
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
	String arguments;
	
	for (int i = 0; i < message.size(); ++i)
	{
		if (message[i].isString()) arguments += "," + message[i].getString();
		else if (message[i].isFloat32()) arguments += "," + String(message[i].getFloat32());
		else if (message[i].isInt32()) arguments += "," + String(message[i].getInt32());
	}

	double time = Time::getMillisecondCounterHiRes() - activationTime;
	String messageText = String(time) + "," + String(currentTrialIndex + 1) + ",";
	
	if (audioFilesArray[currentTrialIndex].exists() && m_player->checkPlaybackStatus())
	{
		messageText += audioFilesArray[currentTrialIndex].getFileName() + "," + message.getAddressPattern().toString() + arguments + "\n";
	}
	else
	{
		messageText += "no stimulus present," + message.getAddressPattern().toString() + arguments + "\n";
	}
	
	oscMessageList.add(messageText);
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
		fos << "time,trial_index,stimulus,osc_pattern,et_el,et_az,et_rot,et_dist,et_conf\n";
		for (int i = 0; i < oscMessageList.size(); ++i)
			fos << oscMessageList[i];
	}
}

void AuditoryLocalisation::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == m_player)
	{
		if (!m_player->checkPlaybackStatus() && m_startTest.getToggleState())
		{
			m_nextTrial.triggerClick();
		}
	}
}

String AuditoryLocalisation::getAudioSrcFilePath()
{
	return audioFilesDir.getFullPathName();
}

void AuditoryLocalisation::setAudioSrcFilePath(String filePath)
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
		// saveSettings();
	}
}

void AuditoryLocalisation::indexAudioFiles()
{
	audioFilesArray.clear();
	Array<File> audioFilesInDir;
	
	// create the test file array (visual stimuli)
	audioFilesInDir.clear();
	DirectoryIterator iter1(audioFilesDir, false, "stim_vis_1_*.wav");
	while (iter1.next())
		audioFilesInDir.add(iter1.getFile());

	for (int i = 0; i < 10; ++i)
	{
		std::random_device seed;
		std::mt19937 rng(seed());
		std::shuffle(audioFilesInDir.begin(), audioFilesInDir.end(), rng);
		audioFilesArray.addArray(audioFilesInDir);
	}

	// create the test file array (audio stimuli)
	audioFilesInDir.clear();
	DirectoryIterator iter2(audioFilesDir, false, "stim_vis_0_*.wav");
	while (iter2.next())
		audioFilesInDir.add(iter2.getFile());

	for (int i = 0; i < 20; ++i)
	{
		std::random_device seed;
		std::mt19937 rng(seed());
		std::shuffle(audioFilesInDir.begin(), audioFilesInDir.end(), rng);
		audioFilesArray.addArray(audioFilesInDir);
	}

	// load files to the player
	m_player->clearPlayer();
	for (auto& file : audioFilesArray)
		m_player->cacheFileToPlayer(file.getFullPathName());

	totalTimeOfAudioFiles = 0.0f;
	for (auto& file : audioFilesArray)
	{
		if (auto * reader = formatManager.createReaderFor(file))
		{
			totalTimeOfAudioFiles += reader->lengthInSamples / reader->sampleRate;
			reader->~AudioFormatReader();
		}

	}
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

void AuditoryLocalisation::loadFile()
{
	m_player->loadSourceToTransport(audioFilesArray[currentTrialIndex].getFullPathName());
	m_player->play();

	String filename = m_player->getCurrentSourceFileName();

	String vis = filename.fromFirstOccurrenceOf("vis_", false, false).upToFirstOccurrenceOf("_", false, false);
	String azi = filename.fromFirstOccurrenceOf("azi_", false, false).upToFirstOccurrenceOf("_", false, false);
	String ele = filename.fromFirstOccurrenceOf("ele_", false, false).dropLastCharacters(4);
	sendMsgToLogWindow("vis: " + vis + ", azi: " + azi + ", ele: " + ele + ", file: " + audioFilesArray[currentTrialIndex].getFileName());
	m_oscTxRx->sendOscMessage("/targetVisAzEl", (int)vis.getIntValue(), (float)azi.getFloatValue(), (float) ele.getFloatValue());
}

void AuditoryLocalisation::sendMsgToLogWindow(String message)
{
	currentMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
}
