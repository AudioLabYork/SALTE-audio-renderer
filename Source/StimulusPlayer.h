/*
  ==============================================================================

    StimulusPlayer.h
    Created: 9 Jul 2019 11:33:50am
    Author:  TR

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTransceiver.h"


class StimulusPlayer    :   public Component,
                            public AudioSource,
                            public ChangeBroadcaster,
                            public OSCReceiver,
                            public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>,
                            private Button::Listener,
                            private ChangeListener,
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
    
    String logWindowMessage;
    StringArray filePathList; // wave file paths
    StringArray fileIdList; // file-ids
    
    // METHODS
    void createFilePathList(String configFilePath);
    void loadFileIntoTransport(const File& audioFile);
    void sendMsgToLogWindow(String message);
    
    // OSC
    oscTransceiver remoteInterfaceTxRx; // osc object to communicate with the user interface (Unity, iPad, ...)
    void oscMessageReceived(const OSCMessage& message) override;
    
    //EDITOR
    void buttonClicked(Button* buttonThatWasClicked) override;
    void changeListenerCallback(ChangeBroadcaster* source) override;
    void timerCallback() override;
    
    void browseForConfigFile();
    void createStimuliTriggerButtons();
    
    TextButton openButton, playButton, stopButton;
    Label loadedFileName, playbackHeadPosition;
    OwnedArray<TextButton> triggerStimuliButtonArray;
    int numberOfStimuli;
    TextEditor logWindow;

private:
    ScopedPointer<AudioFormatReaderSource> currentAudioFileSource;
    TimeSliceThread readAheadThread;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StimulusPlayer)
};

String returnHHMMSS(double lengthInSeconds)
{
    int hours = (int)lengthInSeconds / (60 * 60);
    int minutes = ((int)lengthInSeconds / 60) % 60;
    int seconds = ((int)lengthInSeconds) % 60;
    int millis = floor((lengthInSeconds - floor(lengthInSeconds)) * 100);
    // DBG(String(millis));
    String output = String(hours).paddedLeft('0', 2) + ":" +
    String(minutes).paddedLeft('0', 2) + ":" +
    String(seconds).paddedLeft('0', 2) + "." +
    String(millis).paddedLeft('0', 2);
    return output;
};