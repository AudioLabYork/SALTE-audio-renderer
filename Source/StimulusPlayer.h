#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
// #include "OscTransceiver.h"


class StimulusPlayer    :   public Component,
                            public AudioSource,
                            public ChangeBroadcaster,
                            private Button::Listener,
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
    
    AudioTransportSource transportSource;
    AudioFormatManager formatManager;
    File currentlyLoadedFile;
    
    StringArray filePathList; // wave file paths
    StringArray fileIdList; // file-ids
    
    // METHODS
    void createFilePathList(String configFilePath);
    void loadFileIntoTransport(const File& audioFile);
    void sendMsgToLogWindow(String message);

    //EDITOR
    void buttonClicked(Button* buttonThatWasClicked) override;
    void timerCallback() override;
    
    void createStimuliTriggerButtons();
    
    TextButton openButton, playButton, stopButton;
    Label loadedFileName, playbackHeadPosition;
    OwnedArray<TextButton> triggerStimuliButtonArray;
    int numberOfStimuli;
    
    String currentMessage;


private:
    ScopedPointer<AudioFormatReaderSource> currentAudioFileSource;
    TimeSliceThread readAheadThread;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StimulusPlayer)
};
