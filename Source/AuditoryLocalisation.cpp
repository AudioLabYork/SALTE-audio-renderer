#include "AuditoryLocalisation.h"

AuditoryLocalisation::AuditoryLocalisation()
					: m_oscTxRx(nullptr)
					, m_player(nullptr)
					, m_renderer(nullptr)
{
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

	m_confirmPointer.setButtonText("Confirm Pointer Direction");
	m_confirmPointer.addListener(this);
	addAndMakeVisible(m_confirmPointer);

	// osc logging
	startTimerHz(60);

	m_saveLogButton.setButtonText("Save Log");
	m_saveLogButton.addListener(this);
	m_saveLogButton.setEnabled(false);
	addAndMakeVisible(m_saveLogButton);

	addAndMakeVisible(messageCounter);

	// load settings
	initSettings();
	if (TestSessionFormSettings.getUserSettings()->getBoolValue("loadSettingsFile"))
	{
		loadSettings();
	}
}

AuditoryLocalisation::~AuditoryLocalisation()
{
	saveSettings();
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
	g.drawText("Number of files: " + String(audioFilesArray.size()) + ", total length (s): " + String(totalTimeOfAudioFiles,2), 180, 50, 440, 25, Justification::centredLeft);
	g.drawText("Current trial: " + String(currentTrialIndex + 1) + " of " + String(audioFilesArray.size()), 180, 80, 440, 25, Justification::centredLeft);
}
void AuditoryLocalisation::resized()
{
	m_chooseStimuliFolder.setBounds(20, 20, 150, 25);
	m_startTest.setBounds(20, 50, 150, 25);
	m_prevTrial.setBounds(20, 420, 100, 25);
	m_nextTrial.setBounds(140, 420, 100, 25);
	m_confirmPointer.setBounds(320, 320, 150, 25);

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
		else m_startTest.triggerClick();
	}
	else if (buttonThatWasClicked == &m_confirmPointer)
	{

	}
	else if (buttonThatWasClicked == &m_saveLogButton)
	{
		if (oscMessageList.size() > 0) saveLog();
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
	String messageText = String(time) + ",";
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
	File logFile;
	FileChooser fc("Select or create results export file...",
		File::getCurrentWorkingDirectory(),
		"*.csv",
		true);

	if (fc.browseForFileToSave(true))
	{
		logFile = fc.getResult();


		if (!logFile.exists())
		{
			logFile.create();
			// logFile.replaceWithText("time,address,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12\n");
		}

		FileOutputStream fos(logFile);
		for (int i = 0; i < oscMessageList.size(); ++i)
		{
			fos << oscMessageList[i];
		}
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

void AuditoryLocalisation::selectSrcPath()
{
	FileChooser fc("Select the stimuli folder...",
		File::getSpecialLocation(File::userHomeDirectory));
	if (fc.browseForDirectory())
	{
		audioFilesDir = fc.getResult();
		indexAudioFiles();
		saveSettings();
	}
}

void AuditoryLocalisation::indexAudioFiles()
{
	Array<File> audioFilesInDir;
	audioFilesInDir.clear();

	DirectoryIterator iter(audioFilesDir, true, "*.wav");
	while (iter.next())
	{
		File theFileItFound(iter.getFile());
		audioFilesInDir.add(theFileItFound);
	}

	// create the test audio file array
	audioFilesArray.clear();
	for (int i = 0; i < 10; ++i)
	{
		// shuffle the audio file array
		std::random_device seed;
		std::mt19937 rng(seed());
		std::shuffle(audioFilesInDir.begin(), audioFilesInDir.end(), rng);

		audioFilesArray.addArray(audioFilesInDir);
	}

	// process the audio file array
	if (audioFilesArray.size() > 0)
	{
		AudioFormatManager formatManager;
		formatManager.registerBasicFormats();
		for (int i = 0; i < audioFilesArray.size(); ++i)
		{
			auto* reader = formatManager.createReaderFor(audioFilesArray[i]);
			totalTimeOfAudioFiles += reader->lengthInSamples / reader->sampleRate;
			reader->~AudioFormatReader();
		}
	}
}

void AuditoryLocalisation::loadFile()
{
	m_player->loadFileIntoTransport(audioFilesArray[currentTrialIndex].getFullPathName());
	m_player->play();
}

void AuditoryLocalisation::sendMsgToLogWindow(String message)
{
	currentMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
}

void AuditoryLocalisation::initSettings()
{
	PropertiesFile::Options options;
	options.applicationName = "SALTELocalisationTestSettings";
	options.filenameSuffix = ".conf";
	options.osxLibrarySubFolder = "Application Support";
	options.folderName = File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile).getParentDirectory().getFullPathName();
	options.storageFormat = PropertiesFile::storeAsXML;
	TestSessionFormSettings.setStorageParameters(options);
}

void AuditoryLocalisation::loadSettings()
{
	audioFilesDir = TestSessionFormSettings.getUserSettings()->getValue("audioFilesSrcPath");
	indexAudioFiles();
}

void AuditoryLocalisation::saveSettings()
{
	TestSessionFormSettings.getUserSettings()->setValue("audioFilesSrcPath", audioFilesDir.getFullPathName());
	TestSessionFormSettings.getUserSettings()->setValue("loadSettingsFile", true);
}