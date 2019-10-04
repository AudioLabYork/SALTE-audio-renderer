#include "../JuceLibraryCode/JuceHeader.h"
#include "StimulusPlayer.h"

StimulusPlayer::StimulusPlayer() :  state(Stopped),
                                    readAheadThread("transport read ahead"),
                                    currentTSIndex(0),
									m_shouldShowTest(true)

						
{
	formatManager.registerBasicFormats();
    readAheadThread.startThread(3);
    
    //EDITOR
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open Ambisonic audio file");
    openButton.addListener(this);
    
    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
	playButton.setToggleState(false, NotificationType::dontSendNotification);
	playButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
	playButton.setColour(TextButton::buttonOnColourId, Colours::red);
	playButton.setEnabled(false);
    playButton.addListener(this);
    
    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
	stopButton.setToggleState(true, NotificationType::dontSendNotification);
    stopButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
	stopButton.setColour(TextButton::buttonOnColourId, Colours::red);
	stopButton.setEnabled(false);
    stopButton.addListener(this);

	addAndMakeVisible(&loopButton);
	loopButton.setButtonText("Loop");
	loopButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
	loopButton.setColour(TextButton::buttonOnColourId, Colours::green);
	loopButton.setClickingTogglesState(true);
	loopButton.setEnabled(false);
	loopButton.addListener(this);

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

	gainSlider.setSliderStyle(Slider::LinearVertical);
	gainSlider.setRange(-48, 24, 0.1);
	gainSlider.setValue(0);
	gainSlider.setDoubleClickReturnValue(true, 0);
	gainSlider.setTextBoxIsEditable(true);
	gainSlider.setTextValueSuffix(" dB");
	gainSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 100, 25);
	gainSlider.addListener(this);
	addAndMakeVisible(gainSlider);
	gainSliderLabel.setText("Gain", dontSendNotification);
	gainSliderLabel.setJustificationType(Justification::centredBottom);
	gainSliderLabel.attachToComponent(&gainSlider, false);
	addAndMakeVisible(gainSliderLabel);

	transportSlider.setSliderStyle(Slider::TwoValueHorizontal);
	transportSlider.setRange(0, 1);
	transportSlider.setMinAndMaxValues(0, 1);
	transportSlider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
	transportSlider.addListener(this);
	addAndMakeVisible(transportSlider);

	addAndMakeVisible(pt);

    startTimerHz(60);
}

StimulusPlayer::~StimulusPlayer()
{
	for (int i = 0; i < transportSourceArray.size(); ++i)
		transportSourceArray[i]->setSource(nullptr);
}

void StimulusPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	m_samplesPerBlockExpected = samplesPerBlockExpected;
	m_sampleRate = sampleRate;
}

void StimulusPlayer::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
	if (audioFileSourceArray[currentTSIndex] == nullptr || cachingLock)
    {
		bufferToFill.clearActiveBufferRegion();
		return;
    }

	// check if the playback region has not been reduced at the begining
	if (transportSourceArray[currentTSIndex]->getNextReadPosition() < begOffsetTime * m_sampleRate)
	{
		transportSourceArray[currentTSIndex]->setPosition(begOffsetTime);
	}

	// check if the playback region has not been reduced at the end
	if (transportSourceArray[currentTSIndex]->getNextReadPosition() > transportSourceArray[currentTSIndex]->getTotalLength() - endOffsetTime * m_sampleRate)
	{
		transportSourceArray[currentTSIndex]->setPosition(transportSourceArray[currentTSIndex]->getLengthInSeconds());
	}

	transportSourceArray[currentTSIndex]->getNextAudioBlock(bufferToFill);
	ar.process(*bufferToFill.buffer);
}

void StimulusPlayer::releaseResources()
{
	for (int i = 0; i < transportSourceArray.size(); ++i)
		transportSourceArray[i]->releaseResources();
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
    Rectangle<int> dispRect(270, 10, 450, 150);        // display state
    Rectangle<int> wfRect(10, 170, 710, 120);        // waveform
    
    // DRAW RECTANGLES
    g.setColour(Colours::black);
    g.drawRect(tcRect, 1);
    g.drawRect(dispRect, 1);
    g.drawRect(wfRect, 1);
    
    // TEXT
    g.setFont(Font(14.0f));
    g.setColour(Colours::white);
}

void StimulusPlayer::resized()
{
	openButton.setBounds(20, 20, 230, 25);
	playButton.setBounds(20, 55, 230, 25);
	stopButton.setBounds(20, 90, 230, 25);
	loopButton.setBounds(20, 125, 230, 25);

	loadedFileName.setBounds(280, 20, 500, 25);

	rollSlider.setBounds(320, 75, 300, 25);
	pitchSlider.setBounds(320, 100, 300, 25);
	yawSlider.setBounds(320, 125, 300, 25);

	gainSlider.setBounds(630, 30, 80, 120);

	transportSlider.setBounds(10-3, 295, 710+6, 30);
	playbackHeadPosition.setBounds(280, 45, 500, 25);

	pt.setBounds(10, 170, 710, 120);
}

void StimulusPlayer::changeListenerCallback(ChangeBroadcaster* source)
{
	if (transportSourceArray[currentTSIndex] != nullptr)
	{
		if (source == transportSourceArray[currentTSIndex])
		{
			if (transportSourceArray[currentTSIndex]->isPlaying())
			{
				changeState(Playing);
			}
			else if ((state == Stopping) || (state == Playing))
			{
				if (transportSourceArray[currentTSIndex]->hasStreamFinished() && loopingEnabled)
				{
					transportSourceArray[currentTSIndex]->setPosition(begOffsetTime);
					changeState(Starting);
				}
				else
				{
					changeState(Stopped);
				}
			}
			else if (state == Pausing)
			{
				changeState(Paused);
			}
		}
	}
}

void StimulusPlayer::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &openButton)
    {
        browseForFile();
    }
    else if (buttonThatWasClicked == &playButton)
    {
		play();
    }
    else if (buttonThatWasClicked == &stopButton)
    {
		stop();
    }
	else if (buttonThatWasClicked == &loopButton)
	{
		loop(loopButton.getToggleState());
	}

    repaint();
}

void StimulusPlayer::changeState(TransportState newState)
{
	if (state != newState)
	{
		state = newState;
		switch (state)
		{
		case Stopping:
			if(transportSourceArray[currentTSIndex] != nullptr)
				transportSourceArray[currentTSIndex]->stop();
			break;
		case Stopped:
			if (transportSourceArray[currentTSIndex] != nullptr)
				transportSourceArray[currentTSIndex]->setPosition(begOffsetTime);

			playButton.setButtonText("Play");
			playButton.setToggleState(false, NotificationType::dontSendNotification);
			stopButton.setToggleState(true, NotificationType::dontSendNotification);
			break;
		case Starting:
			if (transportSourceArray[currentTSIndex] != nullptr)
				transportSourceArray[currentTSIndex]->start();
			break;
		case Playing:
			playButton.setButtonText("Pause");
			playButton.setToggleState(true, NotificationType::dontSendNotification);
			stopButton.setToggleState(false, NotificationType::dontSendNotification);
			break;
		case Pausing:
			if (transportSourceArray[currentTSIndex] != nullptr)
				transportSourceArray[currentTSIndex]->stop();
			break;
		case Paused:
			playButton.setButtonText("Play");
			break;
		default:
			break;
		}

		sendChangeMessage();
	}
}

void StimulusPlayer::sliderValueChanged(Slider* slider)
{
	if (slider == &rollSlider || slider == &pitchSlider || slider == &yawSlider)
	{
		float roll = rollSlider.getValue();
		float pitch = pitchSlider.getValue();
		float yaw = yawSlider.getValue();
		ar.updateEulerRPY(roll, pitch, yaw);
	}
	else if (slider == &gainSlider)
	{
		setGain(gainSlider.getValue());
	}
	else if (slider == &transportSlider)
	{
		if (transportSourceArray[currentTSIndex] != nullptr)
		{
			begOffsetTime = transportSourceArray[currentTSIndex]->getLengthInSeconds() * transportSlider.getMinValue();
			endOffsetTime = transportSourceArray[currentTSIndex]->getLengthInSeconds() * (1 - transportSlider.getMaxValue());
		}
	}

}

void StimulusPlayer::timerCallback()
{
	if (transportSourceArray[currentTSIndex] != nullptr)
	{
		double currentPosition = transportSourceArray[currentTSIndex]->getCurrentPosition();
		double lengthInSeconds = transportSourceArray[currentTSIndex]->getLengthInSeconds();

		// update diplayed times in GUI
		playbackHeadPosition.setText("Time: " + returnHHMMSS(currentPosition) + " / " + returnHHMMSS(lengthInSeconds), dontSendNotification);

		pt.setPlaybackCursor(currentPosition / lengthInSeconds);
		pt.setPlaybackOffsets(begOffsetTime / lengthInSeconds, endOffsetTime / lengthInSeconds);
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

void StimulusPlayer::loadFile(String filepath)
{
	File audiofile(filepath);

	if (audiofile.existsAsFile())
		loadFileIntoTransport(audiofile);
	else
		sendMsgToLogWindow("Can't load file: " + filepath);
}

void StimulusPlayer::unloadFileFromTransport()
{
	if(transportSourceArray[currentTSIndex] != nullptr)
		transportSourceArray[currentTSIndex]->stop();

	pt.clearThumbnail();
	loadedFileName.setText("Loaded file: ", dontSendNotification);
	playButton.setEnabled(false);
	stopButton.setEnabled(false);
	loopButton.setEnabled(false);
}

void StimulusPlayer::cacheAudioFile(String filepath)
{
	File audioFile(filepath);

	if (audioFile.existsAsFile() && !fileNameArray.contains(filepath))
	{
		if (AudioFormatReader * reader = formatManager.createReaderFor(audioFile))
		{
			audioFileSourceArray.add(new AudioFormatReaderSource(reader, true));
			
			transportSourceArray.add(new AudioTransportSource);
			transportSourceArray.getLast()->addChangeListener(this);
			transportSourceArray.getLast()->prepareToPlay(m_samplesPerBlockExpected, m_sampleRate);
			transportSourceArray.getLast()->setSource(
				audioFileSourceArray.getLast(),
				24000,					// tells it to buffer this many samples ahead
				&readAheadThread,		// this is the background thread to use for reading-ahead
				reader->sampleRate,     // allows for sample rate correction
				reader->numChannels);   // the maximum number of channels that may need to be played

			fileNameArray.add(filepath);
			numChArray.add(reader->numChannels);
			sendMsgToLogWindow("Cached .wav: " + audioFile.getFileName());
		}
	}
	else if(!audioFile.existsAsFile()) sendMsgToLogWindow("Can't cache file: " + filepath);
}

void StimulusPlayer::clearAudioFileCache()
{
	transportSourceArray.clear();
	audioFileSourceArray.clear();
	fileNameArray.clear();
	numChArray.clear();
	currentTSIndex = 0;
}

void StimulusPlayer::loadFileIntoTransport(const File& audioFile)
{
	unloadFileFromTransport();

	String fullPath = audioFile.getFullPathName();

	if (fileNameArray.contains(fullPath))
	{
		currentlyLoadedFile = audioFile;
		currentTSIndex = fileNameArray.indexOf(fullPath);
		loadedFileChannelCount = numChArray[currentTSIndex];

		// create thumbnail
		pt.createThumbnail(currentlyLoadedFile);
		
		// update GUI label
		loadedFileName.setText("Loaded file: " + currentlyLoadedFile.getFileName(), dontSendNotification);

		// send message to the main log window
		sendMsgToLogWindow("Loaded .wav: " + audioFile.getFileName());

		playButton.setEnabled(true);
		stopButton.setEnabled(true);
		loopButton.setEnabled(true);
	}
}

void StimulusPlayer::setShowTest(bool shouldShow)
{
	m_shouldShowTest = shouldShow;

	openButton.setVisible(!shouldShow);
	loadedFileName.setVisible(!shouldShow);
	rollSlider.setVisible(!shouldShow);
	pitchSlider.setVisible(!shouldShow);
	yawSlider.setVisible(!shouldShow);
	gainSlider.setVisible(!shouldShow);

	resized();
	repaint();
}

void StimulusPlayer::sendMsgToLogWindow(String message)
{
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

void StimulusPlayer::play()
{
	if (transportSourceArray[currentTSIndex] != nullptr)
	{
		if (transportSourceArray[currentTSIndex]->getLengthInSeconds() > 0.0)
		{
			if ((state == Stopped) || (state == Pausing) || (state == Paused))
				changeState(Starting);
			else if (state == Playing)
				changeState(Pausing);
		}
	}
}

void StimulusPlayer::pause()
{
	if (state == Playing)
		changeState(Pausing);
}

void StimulusPlayer::stop()
{
	if (state == Paused)
		changeState(Stopped);
	else if(state == Playing)
		changeState(Stopping);
}

int StimulusPlayer::getNumberOfChannels()
{
	return loadedFileChannelCount;
}

void StimulusPlayer::setGain(float gainInDB)
{
	if (transportSourceArray[currentTSIndex] != nullptr)
	{
		float gain = Decibels::decibelsToGain(gainInDB);
		transportSourceArray[currentTSIndex]->setGain(gain);
		gainSlider.setValue(gainInDB, dontSendNotification);
	}
}

void StimulusPlayer::loop(bool looping)
{
	loopingEnabled = looping;
	
	// if loop gets called from elsewhere (a test component for example)
	if (loopButton.getToggleState() != looping)
		loopButton.setToggleState(looping, dontSendNotification);

	sendChangeMessage();
}

bool StimulusPlayer::checkPlaybackStatus()
{
	if (transportSourceArray[currentTSIndex] == nullptr)
		return false;

	return transportSourceArray[currentTSIndex]->isPlaying();
}

bool StimulusPlayer::checkLoopStatus()
{
	return loopingEnabled;
}

double StimulusPlayer::getPlaybackHeadPosition()
{
	if (transportSourceArray[currentTSIndex] == nullptr)
		return 0.0;

	return transportSourceArray[currentTSIndex]->getCurrentPosition();
}

void StimulusPlayer::setPlaybackHeadPosition(double time)
{
	if (transportSourceArray[currentTSIndex] != nullptr)
		transportSourceArray[currentTSIndex]->setPosition(time);
}

void StimulusPlayer::setPlaybackOffsets(double beg, double end)
{
	begOffsetTime = beg;
	endOffsetTime = end;

	if (transportSourceArray[currentTSIndex] != nullptr)
	{
		double length = transportSourceArray[currentTSIndex]->getLengthInSeconds();
		transportSlider.setMinAndMaxValues(beg / length, 1 - end / length, dontSendNotification);
	}
}
