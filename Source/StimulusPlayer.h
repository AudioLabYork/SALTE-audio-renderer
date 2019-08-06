#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AmbisonicRotation.h"


class StimulusPlayer    :   public Component,
                            public AudioSource,
                            public ChangeBroadcaster,
							private ChangeListener,
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
    
    AudioTransportSource transportSource;
    AudioFormatManager formatManager;
    File currentlyLoadedFile;

	AmbisonicRotation ar;
    
    StringArray filePathList; // wave file paths
    StringArray fileIdList; // file-ids
    
    // METHODS
    void createFilePathList(String configFilePath);
	void browseForFile();
    void loadFileIntoTransport(const File& audioFile);
    void sendMsgToLogWindow(String message);
    String returnHHMMSS(double lengthInSeconds);


    //EDITOR
    void buttonClicked(Button* buttonThatWasClicked) override;
	void sliderValueChanged(Slider* slider) override;
	void timerCallback() override;
    
    void createStimuliTriggerButtons();
    
    TextButton openButton, playButton, stopButton;
    Label loadedFileName, playbackHeadPosition;

	Slider yawSlider, pitchSlider, rollSlider;
	Label yawSliderLabel, pitchSliderLabel, rollSliderLabel;

    OwnedArray<TextButton> triggerStimuliButtonArray;
    int numberOfStimuli = 0;
    
    String currentMessage;


private:
    ScopedPointer<AudioFormatReaderSource> currentAudioFileSource;
    TimeSliceThread readAheadThread;
    
	AudioThumbnailCache thumbnailCache;
	AudioThumbnail thumbnail;

	void paintIfNoFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);
	void paintIfFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StimulusPlayer)
};
