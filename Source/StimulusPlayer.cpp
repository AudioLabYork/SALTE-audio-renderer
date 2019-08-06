#include "../JuceLibraryCode/JuceHeader.h"
#include "StimulusPlayer.h"

StimulusPlayer::StimulusPlayer() :	readAheadThread("transport read ahead"),
									thumbnailCache(10), // maxNumThumbsToStore parameter lets you specify how many previews should be kept in memory at once.
									thumbnail(512, formatManager, thumbnailCache)
						
{
	transportSource.addChangeListener(this);
	thumbnail.addChangeListener(this);
	formatManager.registerBasicFormats();
    readAheadThread.startThread(3);
    
    //EDITOR
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open Ambisonic audio file");
    openButton.addListener(this);
    
    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.setColour(TextButton::buttonColourId, Colours::green);
    playButton.addListener(this);
    
    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.setColour(TextButton::buttonColourId, Colours::red);
    stopButton.addListener(this);
    
    loadedFileName.setText("Loaded file:", dontSendNotification);
    addAndMakeVisible(loadedFileName);
    
    playbackHeadPosition.setText("Time:", dontSendNotification);
    addAndMakeVisible(playbackHeadPosition);

	rollSlider.setSliderStyle(Slider::LinearHorizontal);
	rollSlider.setRange(-180, 180, 0.1);
	rollSlider.setValue(0);
	rollSlider.setDoubleClickReturnValue(true, 0);
	rollSlider.setTextBoxIsEditable(true);
	rollSlider.setTextValueSuffix(" deg");
	rollSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 25);
	rollSlider.addListener(this);
	addAndMakeVisible(rollSlider);
	rollSliderLabel.setText("Roll", dontSendNotification);
	rollSliderLabel.attachToComponent(&rollSlider, true);
	addAndMakeVisible(rollSliderLabel);

	pitchSlider.setSliderStyle(Slider::LinearHorizontal);
	pitchSlider.setRange(-180, 180, 0.1);
	pitchSlider.setValue(0);
	pitchSlider.setDoubleClickReturnValue(true, 0);
	pitchSlider.setTextBoxIsEditable(true);
	pitchSlider.setTextValueSuffix(" deg");
	pitchSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 25);
	pitchSlider.addListener(this);
	addAndMakeVisible(pitchSlider);
	pitchSliderLabel.setText("Pitch", dontSendNotification);
	pitchSliderLabel.attachToComponent(&pitchSlider, true);
	addAndMakeVisible(pitchSliderLabel);

	yawSlider.setSliderStyle(Slider::LinearHorizontal);
	yawSlider.setRange(-180, 180, 0.1);
	yawSlider.setValue(0);
	yawSlider.setDoubleClickReturnValue(true, 0);
	yawSlider.setTextBoxIsEditable(true);
	yawSlider.setTextValueSuffix(" deg");
	yawSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 25);
	yawSlider.addListener(this);
	addAndMakeVisible(yawSlider);
	yawSliderLabel.setText("Yaw", dontSendNotification);
	yawSliderLabel.attachToComponent(&yawSlider, true);
	addAndMakeVisible(yawSliderLabel);
    
    startTimer(30);
}

StimulusPlayer::~StimulusPlayer()
{
    transportSource.setSource(nullptr);
}

void StimulusPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}
void StimulusPlayer::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (currentAudioFileSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    transportSource.getNextAudioBlock(bufferToFill);
	ar.process(*bufferToFill.buffer);
}

void StimulusPlayer::releaseResources()
{
    transportSource.releaseResources();
}

void StimulusPlayer::paint (Graphics& g)
{
    // BACKGROUND
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    // RECTANGULAR OUTLINE
    g.setColour(Colours::black);
    g.drawRect(getLocalBounds(), 1);
    
    // INNER RECTANGLES
    Rectangle<int> tcRect(10, 10, 250, 150);        // manual transport control
    Rectangle<int> dispRect(270, 10, 520, 150);        // display state
    Rectangle<int> wfRect(10, 170, 780, 80);        // waveform
    Rectangle<int> trgRect(10, 260, 780, 115);        // triggers
    
    // DRAW RECTANGLES
    g.setColour(Colours::black);
    g.drawRect(tcRect, 1);
    g.drawRect(dispRect, 1);
    g.drawRect(wfRect, 1);
    g.drawRect(trgRect, 1);
    
    // TEXT
    g.setFont(Font(14.0f));
    g.setColour(Colours::white);

	// PAINT WAVEFORm
	if (thumbnail.getNumChannels() == 0)
		paintIfNoFileLoaded(g, wfRect);
	else
		paintIfFileLoaded(g, wfRect);
}

void StimulusPlayer::resized()
{
    openButton.setBounds(20, 20, 230, 25);
    playButton.setBounds(20, 55, 230, 25);
    stopButton.setBounds(20, 90, 230, 25);
    
    loadedFileName.setBounds(280, 20, 500, 25);
    playbackHeadPosition.setBounds(280, 45, 500, 25);

	rollSlider.setBounds(320, 75, 300, 25);
	pitchSlider.setBounds(320, 100, 300, 25);
	yawSlider.setBounds(320, 125, 300, 25);

        
    if (numberOfStimuli > 0)
    {
        int buttonWidth = 57;
        int buttonHeight = 22;
        int buttonListPositionX = 20 + buttonWidth / 2;
        int buttonListPositionY = 270 + buttonHeight / 2;
        
        for (int i = 0; i < triggerStimuliButtonArray.size(); ++i)
        {
            triggerStimuliButtonArray[i]->setSize(buttonWidth, buttonHeight);
            triggerStimuliButtonArray[i]->setCentrePosition(buttonListPositionX + i % 12 * (buttonWidth + 5),
                                                            buttonListPositionY + floor(i / 12) * (buttonHeight + 5));
        }
    }
}

void StimulusPlayer::changeListenerCallback(ChangeBroadcaster* source)
{
	//if (source == &transportSource) transportSourceChanged();
	if (source == &thumbnail)       repaint();
}

void StimulusPlayer::paintIfNoFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds)
{
	g.setColour(Colours::darkgrey);
	g.fillRect(thumbnailBounds);
	g.setColour(Colours::white);
	g.drawFittedText("No File Loaded", thumbnailBounds, Justification::centred, 1);
}

void StimulusPlayer::paintIfFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds)
{
	g.setColour(Colours::white);
	g.fillRect(thumbnailBounds);

	g.setColour(Colours::red);                                     // [8]

	thumbnail.drawChannel(g,                                      // [9]
		thumbnailBounds,
		0.0,                                    // start time
		thumbnail.getTotalLength(),             // end time
		0,										// channel number
		1.0f);                                  // vertical zoom
}

void StimulusPlayer::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &openButton)
    {
        browseForFile();
    }
    else if (buttonThatWasClicked == &playButton)
    {
        transportSource.setPosition(0);
        transportSource.start();
    }
    else if (buttonThatWasClicked == &stopButton)
    {
        transportSource.stop();
    }
    else if (buttonThatWasClicked->getProperties()["triggerStimuliButton"])
    {
        int buttonIndex = buttonThatWasClicked->getProperties()["buttonIndex"];
        loadFileIntoTransport(File(filePathList[buttonIndex]));
        transportSource.setPosition(0);
        transportSource.start();
    }
    repaint();
}

void StimulusPlayer::sliderValueChanged(Slider* slider)
{
	float roll = rollSlider.getValue();
	float pitch = pitchSlider.getValue();
	float yaw = yawSlider.getValue();

	ar.updateEuler(roll, pitch, yaw);
}

void StimulusPlayer::timerCallback()
{
    double currentPosition = transportSource.getCurrentPosition();
    double lengthInSeconds = transportSource.getLengthInSeconds();
    
    // update diplayed times in GUI
    playbackHeadPosition.setText("Time: " + returnHHMMSS(currentPosition) + " / " + returnHHMMSS(lengthInSeconds), dontSendNotification);
}

void StimulusPlayer::createFilePathList(String configFilePath)
{
	File fileToLoad = File(configFilePath);
	StringArray loadedData;
	loadedData.clear();
	loadedData.addLines(fileToLoad.loadFileAsString());
	if (loadedData[0].startsWith("#ASP#Config#File#"))
	{
		filePathList.clear();
		int header = 10; // number of header lines
		for (int i = header; i < loadedData.size(); ++i)
		{
			StringArray tokens;
			tokens.addTokens(loadedData[i], ",", "\"");
			if (tokens[3].length() != 0)
			{
				filePathList.set(i, tokens[2] + "/" + tokens[3]); // concatenate file path + file name and add the full path to the file path list
				fileIdList.set(i, tokens[0] + tokens[1]); // add the file-id to the file-id list

				// output log
				sendMsgToLogWindow(tokens[0] + tokens[1] + ": " + tokens[3] + " added.");
			}
		}

		// load the first file from the list
		loadFileIntoTransport(File(filePathList[0]));
	}
	else
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon, "Invalid config file.", fileToLoad.getFullPathName());
	}
}

void StimulusPlayer::browseForFile()
{
	FileChooser chooser("Select a Wave file to play...",
		File::getSpecialLocation(File::userHomeDirectory),
		"*.wav", true);

	if (chooser.browseForFileToOpen())
	{
		auto file = chooser.getResult();
		loadFileIntoTransport(file);
	}
}

void StimulusPlayer::loadFileIntoTransport(const File& audioFile)
{
	// unload the previous file source and delete it..
	transportSource.stop();
	transportSource.setSource(nullptr);
	currentAudioFileSource = nullptr;

	AudioFormatReader* reader = formatManager.createReaderFor(audioFile);
	currentlyLoadedFile = audioFile;

	if (reader != nullptr)
	{
		currentAudioFileSource = new AudioFormatReaderSource(reader, true);

		// loading into transport source using a separate thread comes from https://github.com/jonathonracz/AudioFilePlayerPlugin
		// ..and plug it into our transport source
		transportSource.setSource(
			currentAudioFileSource,
			32768,                  // tells it to buffer this many samples ahead
			&readAheadThread,       // this is the background thread to use for reading-ahead
			reader->sampleRate,     // allows for sample rate correction
			reader->numChannels);    // the maximum number of channels that may need to be played
		
		// create thumbnail
		thumbnail.setSource(new FileInputSource(currentlyLoadedFile));
		
		// update GUI label
		loadedFileName.setText("Loaded file: " + currentlyLoadedFile.getFileName(), dontSendNotification);

		// send message to the main log window
		sendMsgToLogWindow("Loaded: " + audioFile.getFileName());
		sendMsgToLogWindow(String(reader->numChannels) + "," +
			String(reader->bitsPerSample) + "," +
			String(reader->sampleRate) + "," +
			String(reader->lengthInSamples) + "," +
			String(reader->lengthInSamples / reader->sampleRate));
	}
}

void StimulusPlayer::sendMsgToLogWindow(String message)
{
	// is it safe? (1/2)
	currentMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
}

String StimulusPlayer::returnHHMMSS(double lengthInSeconds)
{
    int hours = (int)lengthInSeconds / (60 * 60);
    int minutes = ((int)lengthInSeconds / 60) % 60;
    int seconds = ((int)lengthInSeconds) % 60;
    int millis = floor((lengthInSeconds - floor(lengthInSeconds)) * 100);
    
    String output = String(hours).paddedLeft('0', 2) + ":" +
    String(minutes).paddedLeft('0', 2) + ":" +
    String(seconds).paddedLeft('0', 2) + "." +
    String(millis).paddedLeft('0', 2);
    return output;
}

void StimulusPlayer::createStimuliTriggerButtons()
{
        for (int i = 0; i < numberOfStimuli; ++i)
    {
        triggerStimuliButtonArray.add(new TextButton());
        triggerStimuliButtonArray[i]->getProperties().set("triggerStimuliButton", true);
        triggerStimuliButtonArray[i]->getProperties().set("buttonIndex", i);
        triggerStimuliButtonArray[i]->setButtonText(fileIdList[i]);
        triggerStimuliButtonArray[i]->addListener(this);
        addAndMakeVisible(triggerStimuliButtonArray[i]);
    }
    resized();
}
