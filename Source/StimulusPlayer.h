#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AmbisonicRotation.h"
#include "PlayerThumbnail.h"

class StimulusPlayer
	: public Component
	, public AudioSource
	, public ChangeBroadcaster
	, private ChangeListener
	, private Button::Listener
	, private Slider::Listener
	, private Timer
{
public:

	enum TransportState
	{
		Stopping,
		Stopped,
		Starting,
		Playing,
		Pausing,
		Paused
	};

	StringArray TransportStateString
	{
		"Stopping",
		"Stopped",
		"Starting",
		"Playing",
		"Pausing",
		"Paused"
	};

    StimulusPlayer();
    ~StimulusPlayer();
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    void paint (Graphics&) override;
    void resized() override;

	void changeListenerCallback(ChangeBroadcaster* source) override;
	void changeState(TransportState newState);
	void buttonClicked(Button* buttonThatWasClicked) override;
	void sliderValueChanged(Slider* slider) override;
	void timerCallback() override;

	// exposing some playback transport functionality
	void play();
	void pause();
	void stop();
	void loop(bool looping);

	void setGain(const float gainInDB);
	bool getLoopingState();
	bool checkPlaybackStatus();
	bool checkLoopStatus();
	
	double getTotalTimeForCachedFiles() const;
	double getPlaybackHeadPosition();
	void setPlaybackHeadPosition(double time);
	void setPlaybackOffsets(double beg, double end);
	double getPlaybackStartOffset();
	double getPlaybackEndOffset();
	
	void clearPlayer();
	void cacheFileToPlayer(const String& fullPath);
	void loadSourceToTransport(const String& fullPath);
	String getCurrentSourceFileName();

	void setShowTest(bool shouldShow);

	// log window message
    String currentMessage;

	String getAudioFilesDir();
	void setAudioFilesDir(String filePath);
private:
	void browseForFile();
	void sendMsgToLogWindow(String message);
	String returnHHMMSS(double lengthInSeconds);

	TransportState state;
	TimeSliceThread readAheadThread;

	File audioFilesDir;
	std::vector<File> audioSourceFiles;
	std::vector<std::unique_ptr<AudioFormatReaderSource>> audioFormatReaderSources;
	StringArray cachedFileNames;
	AudioTransportSource transportSource;

	int m_samplesPerBlockExpected;
	double m_sampleRate;
	
	AudioFormatManager formatManager;

	AmbisonicRotation rotator;

	bool loopingEnabled;
	double begOffsetTime;
	double endOffsetTime;

	TextButton openButton, playButton, stopButton, loopButton;
	Label loadedFileName, playbackHeadPosition;

	Slider yawSlider, pitchSlider, rollSlider, gainSlider;
	Label yawSliderLabel, pitchSliderLabel, rollSliderLabel, gainSliderLabel;
	Slider transportSlider;

	PlayerThumbnail playerThumbnail;

	bool m_shouldShowTest;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StimulusPlayer)
};
