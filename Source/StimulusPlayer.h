#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AmbisonicRotation.h"


class StimulusPlayer    :   public Component,
                            public AudioSource,
                            public ChangeBroadcaster,
							private ChangeListener,
							public OSCReceiver,
							public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>,
                            private Button::Listener,
							private Slider::Listener,
                            private Timer
{
public:
    StimulusPlayer();
    ~StimulusPlayer();
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    void paint (Graphics&) override;
    void resized() override;

	void changeListenerCallback(ChangeBroadcaster* source) override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void sliderValueChanged(Slider* slider) override;
	void timerCallback() override;

	void oscMessageReceived(const OSCMessage& message) override;

	// exposing some playback transport functionality
	void play();
	void pause();
	void stop();
	int getNumberOfChannels();
	void setGain(float gainInDB);
	void loop(bool looping);
	bool checkPlaybackStatus();
	void loadFile(String filepath);
	double getPlaybackHeadPosition();
	void setPlaybackHeadPosition(double time);
    
	// log window message
    String currentMessage;


private:
	AudioTransportSource transportSource;
	AudioFormatManager formatManager;
	File currentlyLoadedFile;

	AmbisonicRotation ar;

	bool loopingEnabled = false;
	bool isPlaying = false;
	int loadedFileChannelCount = 0;

	StringArray filePathList; // wave file paths
	StringArray fileIdList; // file-ids

	// METHODS
	void browseForFile();
	void loadFileIntoTransport(const File& audioFile);
	void sendMsgToLogWindow(String message);
	String returnHHMMSS(double lengthInSeconds);

	// void browseForConfigFile();
	// void createFilePathList(String configFilePath);
	// void createStimuliTriggerButtons();

	void paintIfNoFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);
	void paintIfFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);

	TextButton openButton, playButton, stopButton;
	Label loadedFileName, playbackHeadPosition;

	Slider yawSlider, pitchSlider, rollSlider;
	Label yawSliderLabel, pitchSliderLabel, rollSliderLabel;
	Slider transportSlider;

	// OwnedArray<TextButton> triggerStimuliButtonArray;
	// int numberOfStimuli = 0;

    std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;
    TimeSliceThread readAheadThread;
    
	AudioThumbnailCache thumbnailCache;
	AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StimulusPlayer)
};
