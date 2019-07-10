#include "MainComponent.h"

MainComponent::MainComponent()
{
    
    // add and make visible the stimulus player object
    addAndMakeVisible(sp);
    sp.addChangeListener(this);

    // set size of the main app window
    setSize (1400, 800);

    // set number of output channels to 2 (binaural rendering case)
    setAudioChannels (0, 2);
    
    // OSC sender and receiver connect
    remoteInterfaceTxRx.connectSender("127.0.0.1", 6000);
    remoteInterfaceTxRx.connectReceiver(9000);
    remoteInterfaceTxRx.addListener(this);
    
    addAndMakeVisible(&openConfigButton);
    openConfigButton.setButtonText("Open config file...");
    openConfigButton.addListener(this);
    
    addAndMakeVisible(logWindow);
    logWindow.setMultiLine(true);
    logWindow.setReadOnly(true);
    logWindow.setCaretVisible(false);
    logWindow.setScrollbarsShown(true);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	// prepare stimulus player object
	sp.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
	// pass the buffer into the stimulus player to be filled with required audio
	sp.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
	// relese resources taken by stimulus player object
    sp.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    Rectangle<int> oscRect(250, 10, 330, 150);        // osc status / vr interface status
    Rectangle<int> tstlogicRect(10, 170, 570, 480);        // test logic component
    Rectangle<int> renderRect(590, 405, 800, 385);     // rendering component
    g.drawRect(oscRect, 1);
    g.drawRect(tstlogicRect, 1);
    g.drawRect(renderRect, 1);
    
}

void MainComponent::resized()
{
    sp.setBounds(590, 10, 800, 385);
    openConfigButton.setBounds(10, 10, 230, 25);
    logWindow.setBounds(10, 660, 570, 130);
}

void MainComponent::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &openConfigButton)
    {
        browseForConfigFile();
    }
    repaint();
}

// load config file
void MainComponent::browseForConfigFile()
{
    
#if JUCE_MODAL_LOOPS_PERMITTED
    const bool useNativeVersion = true;
    FileChooser fc("Choose a file to open...",
                   File::getCurrentWorkingDirectory(),
                   "*.csv",
                   useNativeVersion);
    
    if (fc.browseForFileToOpen())
    {
        File chosenFile = fc.getResult();
        
        // configure sample player
        sp.createFilePathList(chosenFile.getFullPathName());
        sp.numberOfStimuli = sp.filePathList.size();
        sp.createStimuliTriggerButtons(); // only for test
    }
#endif
}

// OSC
void MainComponent::oscMessageReceived(const OSCMessage& message)
{
    // DIRECT OSC CONTROL OF STIMULUS PLAYER
    if (message.size() == 1 && message.getAddressPattern() == "/stimulus" && message[0].isInt32())
    {
        sp.loadFileIntoTransport(File(sp.filePathList[message[0].getInt32()]));
    }
    
    if (message.size() == 1 && message.getAddressPattern() == "/transport" && message[0].isString())
    {
        if (message[0].getString() == "play")
        {
            sp.transportSource.setPosition(0);
            sp.transportSource.start();
        }
        
        if (message[0].getString() == "stop")
        {
            sp.transportSource.stop();
        }
    }
}

// LOG WINDOW
void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
    if(source == &sp)
    {
        // is it safe? (2/2)
        logWindowMessage += sp.currentMessage;
        sp.currentMessage.clear();
    }
    
    logWindow.setText(logWindowMessage);
    logWindow.moveCaretToEnd();
}
