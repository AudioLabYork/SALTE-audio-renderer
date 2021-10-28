#include "../JuceLibraryCode/JuceHeader.h"
#include "StimulusPlayer.h"

StimulusPlayer::StimulusPlayer()
	: m_state(StimulusPlayer::Stopped)
	, readAheadThread("transport read ahead")
	, m_samplesPerBlockExpected(0)
	, m_sampleRate(0.0)
	, m_loopingEnabled(false)
	, begOffsetTime(0.0)
	, endOffsetTime(0.0)
	, m_shouldShowTest(true)
{
	formatManager.registerBasicFormats();
	readAheadThread.startThread(3);

	addAndMakeVisible(&openButton);
	openButton.setButtonText("Select audio file...");
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

	addAndMakeVisible(playerThumbnail);

	transportSource.addChangeListener(this);

	startTimerHz(60);
}

StimulusPlayer::~StimulusPlayer()
{
	transportSource.setSource(nullptr);
}

void StimulusPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	m_samplesPerBlockExpected = samplesPerBlockExpected;
	m_sampleRate = sampleRate;

	transportSource.prepareToPlay(m_samplesPerBlockExpected, m_sampleRate);
}

void StimulusPlayer::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
	// check if the playback region has been reduced at the begining
	if (transportSource.getNextReadPosition() < begOffsetTime * m_sampleRate)
		transportSource.setNextReadPosition(begOffsetTime * m_sampleRate);

	// check if the playback region has been reduced at the end
	if (endOffsetTime > 0.0f && transportSource.getNextReadPosition() > transportSource.getTotalLength() - endOffsetTime * m_sampleRate)
	{
		if (m_loopingEnabled) //this looping method doesn't play samples remaining in the last buffer before jumping to the beginning, but it's fast
		{
			transportSource.setNextReadPosition(begOffsetTime * m_sampleRate);
		}
		else
		{
			transportSource.setNextReadPosition(transportSource.getTotalLength());
		}
	}
	else if (endOffsetTime == 0.0f && m_loopingEnabled && transportSource.getNextReadPosition() > transportSource.getTotalLength() - m_samplesPerBlockExpected)
	{
		transportSource.setNextReadPosition(begOffsetTime * m_sampleRate);
	}

	transportSource.getNextAudioBlock(bufferToFill);
	rotator.process(*bufferToFill.buffer);
}

void StimulusPlayer::releaseResources()
{
	transportSource.releaseResources();
}

void StimulusPlayer::paint(Graphics& g)
{
	// background
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	// outline
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

	// inner outlines
	Rectangle<int> tcRect(10, 10, 250, 150); // manual transport control
	Rectangle<int> dispRect(270, 10, 450, 150); // display state
	Rectangle<int> wfRect(10, 170, 710, 120); // waveform

	// draw inner outlines
	g.setColour(Colours::black);
	g.drawRect(tcRect, 1);
	g.drawRect(dispRect, 1);
	g.drawRect(wfRect, 1);

	// text font and colour
	g.setFont(Font(14.0f));
	g.setColour(Colours::white);
}

void StimulusPlayer::resized()
{
	openButton.setBounds(20, 20, 230, 25);
	playButton.setBounds(20, 55, 230, 25);
	stopButton.setBounds(20, 90, 230, 25);
	loopButton.setBounds(20, 125, 230, 25);

	loadedFileName.setBounds(280, 20, 400, 25);

	rollSlider.setBounds(320, 75, 300, 25);
	pitchSlider.setBounds(320, 100, 300, 25);
	yawSlider.setBounds(320, 125, 300, 25);

	gainSlider.setBounds(630, 30, 80, 120);

	transportSlider.setBounds(10 - 3, 295, 710 + 6, 30);
	playbackHeadPosition.setBounds(280, 45, 500, 25);

	playerThumbnail.setBounds(10, 170, 710, 120);
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

String StimulusPlayer::getAudioFilesDir()
{
	return audioFilesDir.getFullPathName();
}

void StimulusPlayer::setAudioFilesDir(String filePath)
{
	if (File(filePath).exists())
	{
		audioFilesDir = filePath;
	}
}

void StimulusPlayer::sliderValueChanged(Slider* slider)
{
	if (slider == &rollSlider || slider == &pitchSlider || slider == &yawSlider)
	{
		float roll = static_cast<float>(rollSlider.getValue());
		float pitch = static_cast<float>(pitchSlider.getValue());
		float yaw = static_cast<float>(yawSlider.getValue());
		
		rotator.updateEulerRPY(roll, pitch, yaw);
	}
	else if (slider == &gainSlider)
	{
		float gain = static_cast<float>(gainSlider.getValue());
		
		setGain(gain);
	}
	else if (slider == &transportSlider)
	{
		begOffsetTime = transportSource.getLengthInSeconds() * transportSlider.getMinValue();
		endOffsetTime = transportSource.getLengthInSeconds() * (1.0 - transportSlider.getMaxValue());
	}
}

void StimulusPlayer::timerCallback()
{
	double currentPosition = transportSource.getCurrentPosition();
	double lengthInSeconds = transportSource.getLengthInSeconds();

	// update diplayed times in GUI
	playbackHeadPosition.setText("Time: " + returnHHMMSS(currentPosition) + " / " + returnHHMMSS(lengthInSeconds), dontSendNotification);

	playerThumbnail.setPlaybackCursor(currentPosition / lengthInSeconds);
	playerThumbnail.setPlaybackOffsets(begOffsetTime / lengthInSeconds, endOffsetTime / lengthInSeconds);
}

void StimulusPlayer::browseForFile()
{
	auto chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...", audioFilesDir,"*.wav");
	auto chooserFlags = juce::FileBrowserComponent::openMode
		| juce::FileBrowserComponent::canSelectFiles;

	chooser->launchAsync(chooserFlags, [this](const FileChooser& fc)
		{
			File file = fc.getResult();
			if (file != File{})
			{
				audioFilesDir = file.getParentDirectory();
				clearPlayer();
				cacheFileToPlayer(file.getFullPathName());
				loadSourceToTransport(file.getFullPathName());
			}
		});


	//FileChooser chooser("Select a Wave file to play...",
	//	audioFilesDir,
	//	"*.wav", true);

	//if (chooser.browseForFileToOpen())
	//{
	//	
	//	audioFilesDir = file.getParentDirectory();
	//	clearPlayer();
	//	cacheFileToPlayer(file.getFullPathName());
	//	loadSourceToTransport(file.getFullPathName());
	//}
}

void StimulusPlayer::clearPlayer()
{
	transportSource.setSource(nullptr);
	audioSourceFiles.clear();
	cachedFileNames.clear();
	audioFormatReaderSources.clear();
	playerThumbnail.clearThumbnail();
	sendMsgToLogWindow("Cache cleared");
}

void StimulusPlayer::cacheFileToPlayer(const String& fullPath)
{
	File audiofile(fullPath);

	if (audiofile.existsAsFile() && !cachedFileNames.contains(fullPath))
	{
		if (AudioFormatReader * reader = formatManager.createReaderFor(audiofile))
		{
			std::unique_ptr<AudioFormatReaderSource> audioFormatReaderSource = std::make_unique<AudioFormatReaderSource>(reader, true);
			audioSourceFiles.push_back(audiofile);
			audioFormatReaderSources.push_back(std::move(audioFormatReaderSource));
			cachedFileNames.add(fullPath);
			sendMsgToLogWindow("Cached: " + audiofile.getFileName());
		}
	}
}

void StimulusPlayer::loadSourceToTransport(const String& fullPath)
{
	if (cachedFileNames.contains(fullPath, false))
	{
		const int index = cachedFileNames.indexOf(fullPath, false, 0);
		auto source_ptr = audioFormatReaderSources[index].get();
		auto source_fs = audioFormatReaderSources[index]->getAudioFormatReader()->sampleRate;
		auto source_chnum = audioFormatReaderSources[index]->getAudioFormatReader()->numChannels;
		auto source_bitdepth = audioFormatReaderSources[index]->getAudioFormatReader()->bitsPerSample;

		transportSource.setSource(
			source_ptr,
			32768, // tells it to buffer this many samples ahead
			&readAheadThread, // this is the background thread to use for reading-ahead
			source_fs, // allows for sample rate correction
			source_chnum); // the maximum number of channels that may need to be played

		playerThumbnail.createThumbnail(audioSourceFiles[index]);
		loadedFileName.setText(audioSourceFiles[index].getFileName(), dontSendNotification);
		sendMsgToLogWindow("Loaded: " + audioSourceFiles[index].getFileName() + ", " + String(source_chnum) + " channels, " + String(source_fs) + "Hz, " + String(source_bitdepth) + "bit");

		playButton.setEnabled(true);
		stopButton.setEnabled(true);
		loopButton.setEnabled(true);
	}
	else
	{
		sendMsgToLogWindow("Missing: " + fullPath);
	}
}

String StimulusPlayer::getCurrentSourceFileName()
{
	return loadedFileName.getText();
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

void StimulusPlayer::play()
{
	if (transportSource.getLengthInSeconds() > 0.0)
	{
		if ((m_state == Stopped) || (m_state == Paused) || (m_state == Pausing))
			changeState(Starting);
		//else if (m_state == Playing)
		//	changeState(Pausing);
	}
}

void StimulusPlayer::pause()
{
	if (m_state == Playing)
		changeState(Pausing);
}

void StimulusPlayer::stop()
{
	if (m_state == Paused)
		changeState(Stopped);
	else
		changeState(Stopping);
}

void StimulusPlayer::loop(bool looping)
{
	m_loopingEnabled = looping;

	// if loop gets called from elsewhere (a test component for example)
	if (loopButton.getToggleState() != looping)
		loopButton.setToggleState(looping, dontSendNotification);
}

void StimulusPlayer::changeState(TransportState newState)
{
	if (m_state != newState)
	{
		m_state = newState;
		switch (m_state)
		{
		case Stopped:
			playButton.setButtonText("Play");
			playButton.setToggleState(false, NotificationType::dontSendNotification);
			stopButton.setToggleState(true, NotificationType::dontSendNotification);
			transportSource.setPosition(begOffsetTime);
			sendChangeMessage(); // to inform test component that playback has stopped
			break;
		case Starting:
			transportSource.start();
			break;
		case Playing:
			playButton.setButtonText("Pause");
			playButton.setToggleState(true, NotificationType::dontSendNotification);
			stopButton.setToggleState(false, NotificationType::dontSendNotification);
			break;
		case Pausing:
			transportSource.stop();
			break;
		case Paused:
			playButton.setButtonText("Resume");
			break;
		case Stopping:
			transportSource.stop();
			break;
		default:
			// unknown state
			jassertfalse;
			break;
		}
		//sendMsgToLogWindow("State: " + TransportStateString[m_state]);
	}
}

void StimulusPlayer::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == &transportSource)
	{
		if (transportSource.isPlaying())
			changeState(Playing);
		else if (m_state == Playing)
			changeState(Stopped);
			//if (m_loopingEnabled) //this looping method will play all samples but involves stop and start
			//{
			//	transportSource.setPosition(begOffsetTime);
			//	transportSource.start();
			//}
			//else
			//{
			//	changeState(Stopped);
			//}
		else if (m_state == Stopping)
			changeState(Stopped);
		else if (m_state == Pausing)
			changeState(Paused);
	}
}

void StimulusPlayer::setGain(const float gainInDB)
{
	transportSource.setGain(Decibels::decibelsToGain(gainInDB));
	gainSlider.setValue(gainInDB, dontSendNotification);
}

bool StimulusPlayer::getLoopingState()
{
	return m_loopingEnabled;
}

bool StimulusPlayer::checkPlaybackStatus()
{
	return transportSource.isPlaying();
}

bool StimulusPlayer::checkLoopStatus()
{
	return m_loopingEnabled;
}

double StimulusPlayer::getTotalTimeForCachedFiles() const
{
	if (audioFormatReaderSources.size() <= 0)
		return 0;

	int totalTime = 0;

	for (const auto& reader : audioFormatReaderSources)
		totalTime += reader->getTotalLength();

	return totalTime / audioFormatReaderSources[0]->getAudioFormatReader()->sampleRate;
}

double StimulusPlayer::getPlaybackHeadPosition()
{
	return transportSource.getCurrentPosition();
}

void StimulusPlayer::setPlaybackHeadPosition(double time)
{
	transportSource.setPosition(time);
}

void StimulusPlayer::setPlaybackOffsets(double beg, double end)
{
	if (beg > end)
	{
		// the beginning must come before the end
		jassertfalse;
		return;
	}

	begOffsetTime = beg;
	endOffsetTime = end;

	double length = transportSource.getLengthInSeconds();

	if (length > 0.0)
		transportSlider.setMinAndMaxValues(beg / length, 1.0 - end / length, dontSendNotification);
	else
		transportSlider.setMinAndMaxValues(0.0, 1.0, dontSendNotification);

	sendChangeMessage();
}

double StimulusPlayer::getPlaybackStartOffset()
{
	return begOffsetTime;
}

double StimulusPlayer::getPlaybackEndOffset()
{
	return endOffsetTime;
}
