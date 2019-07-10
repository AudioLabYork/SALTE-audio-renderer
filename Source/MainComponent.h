#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"


class MainComponent   :     public AudioAppComponent,
                            private Button::Listener,
                            public OSCReceiver,
                            public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>,
                            public ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* buttonThatWasClicked) override;

    void browseForConfigFile();
    
    // OSC
    oscTransceiver remoteInterfaceTxRx; // osc object to communicate with the user interface (Unity, iPad, ...)
    void oscMessageReceived(const OSCMessage& message) override;

    // LOG WINDOW UPDATE
    void changeListenerCallback(ChangeBroadcaster* source) override;

private:
    //==============================================================================
    // Your private member variables go here...
    
    StimulusPlayer sp;
    
    String logWindowMessage;
    TextEditor logWindow;
    
    TextButton openConfigButton;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
