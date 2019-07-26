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
    
    // configure MUSHRA
    configureMushra();
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
	// pass the buffer to the binaural rendering object to replace ambisonic signals with binaural audio
	// br.getNextAudioBlock(bufferToFill)
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
    
    // fit mushra interface
    mc.setBounds(10, 170, 570, 480);
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

void MainComponent::configureMushra()
{
    int numberOfRegions = 4;
    for(int i = 0; i < numberOfRegions; ++i)
    {
        mc.regionArray.add(new SampleRegion());
        mc.regionArray[i]->dawStartTime = 0.0f; //markerTimeArray[i * 2];         // 0 2 4 6
        mc.regionArray[i]->dawStopTime = 5.0f;  //markerTimeArray[(i * 2) + 1];    // 1 3 5 7
        mc.regionArray[i]->calculateStartEndTimes();
    }
    
    int numberOfSamplesPerRegion = 8;
    mc.numberOfSamplesPerRegion = numberOfSamplesPerRegion;
    

//    mc.connectOsc(dawIp, clientIp, dawTxPort, dawRxPort, clientTxPort, clientRxPort);
    mc.createGui();
    addAndMakeVisible(mc);
    repaint();
}

// OSC
void MainComponent::oscMessageReceived(const OSCMessage& message)
{
    // DIRECT OSC CONTROL OF STIMULUS PLAYER

	// load file from the list (index received by osc)
    if (message.size() == 1 && message.getAddressPattern() == "/stimulus" && message[0].isInt32())
    {
        sp.loadFileIntoTransport(File(sp.filePathList[message[0].getInt32()]));
    }

	// load file from path (file path received by osc)
	if (message.size() == 1 && message.getAddressPattern() == "/stimulus" && message[0].isString())
	{
		sp.loadFileIntoTransport(File(message[0].getString()));
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
