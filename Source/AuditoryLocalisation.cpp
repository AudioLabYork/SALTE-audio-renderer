#include "AuditoryLocalisation.h"

AuditoryLocalisation::AuditoryLocalisation()
					: m_oscTxRx(nullptr)
					, m_player(nullptr)
					, m_renderer(nullptr)
{
	g_chooseStimuliFolder.setButtonText("Select Stimuli Folder");
	g_chooseStimuliFolder.addListener(this);
	addAndMakeVisible(g_chooseStimuliFolder);

	g_startTest.setButtonText("Start Test");
	g_startTest.addListener(this);
	addAndMakeVisible(g_startTest);

	g_prevTrial.setButtonText("Previous Trial");
	g_prevTrial.addListener(this);
	addAndMakeVisible(g_prevTrial);

	g_nextTrial.setButtonText("Next Trial");
	g_nextTrial.addListener(this);
	addAndMakeVisible(g_nextTrial);

	g_confirmPointer.setButtonText("Confirm Pointer Direction");
	g_confirmPointer.addListener(this);
	addAndMakeVisible(g_confirmPointer);

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

void AuditoryLocalisation::init(StimulusPlayer* player, BinauralRenderer* renderer)
{
	m_renderer = renderer;
	m_player = player;
	m_player->addChangeListener(this);
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
	g_chooseStimuliFolder.setBounds(20, 20, 150, 25);
	g_startTest.setBounds(20, 50, 150, 25);
	g_prevTrial.setBounds(20, 420, 100, 25);
	g_nextTrial.setBounds(140, 420, 100, 25);
	g_confirmPointer.setBounds(320, 320, 150, 25);

}
void AuditoryLocalisation::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &g_chooseStimuliFolder)
	{
		selectSrcPath();
	}
	else if (buttonThatWasClicked == &g_startTest)
	{
		loadFile();
	}
	else if (buttonThatWasClicked == &g_prevTrial)
	{
		if(currentTrialIndex > 0)
		{
			currentTrialIndex--;
			loadFile();
		}
	}
	else if (buttonThatWasClicked == &g_nextTrial)
	{
		if (currentTrialIndex < audioFilesArray.size() - 1)
		{
			currentTrialIndex++;
			loadFile();
		}
	}
	else if (buttonThatWasClicked == &g_confirmPointer)
	{

	}
	repaint();
}

void AuditoryLocalisation::oscMessageReceived(const OSCMessage& message)
{

}

void AuditoryLocalisation::changeListenerCallback(ChangeBroadcaster* source)
{

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
	audioFilesArray.clear();
	DirectoryIterator iter(audioFilesDir, true, "*.wav");
	while (iter.next())
	{
		File theFileItFound(iter.getFile());
		audioFilesArray.add(theFileItFound);
	}


	if (audioFilesArray.size() > 0)
	{
		// read and verify audio files properties
		bool filesVerified = true;
		AudioFormatManager formatManager;
		formatManager.registerBasicFormats();
		for (int i = 0; i < audioFilesArray.size(); ++i)
		{
			auto* reader = formatManager.createReaderFor(audioFilesArray[i]);
			totalTimeOfAudioFiles += reader->lengthInSamples / reader->sampleRate;
			reader->~AudioFormatReader();
		}

		// shuffle the audio file array
		std::random_device seed;
		std::mt19937 rng(seed());
		std::shuffle(audioFilesArray.begin(), audioFilesArray.end(), rng);
	}
}

void AuditoryLocalisation::loadFile()
{
	m_player->loadFileIntoTransport(audioFilesArray[currentTrialIndex].getFullPathName());
	m_player->play(); // sometimes play is not triggering (called too soon?)
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