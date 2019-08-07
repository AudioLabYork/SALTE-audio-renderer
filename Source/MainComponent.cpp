#include "MainComponent.h"

MainComponent::MainComponent()
	: as(deviceManager)
	, m_maxSamplesPerBlock(0)
{
    // add and make visible the stimulus player object
    addAndMakeVisible(sp);
    sp.addChangeListener(this);

	// setup binaural renderer
	br.init();
	br.addChangeListener(this);

	addAndMakeVisible(br);

	File sourcePath(File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("SALTE"));
	
	Result res = sourcePath.createDirectory();
	if (res.wasOk())
	{
		Logger::outputDebugString("application directory created successfully");
	}

	// set size of the main app window
    setSize (1400, 800);

    // set number of output channels to 2 (binaural rendering case)
    setAudioChannels (0, 2);

	// OSC labels
	clientTxIpLabel.setEditable(false, true, false);
	clientTxPortLabel.setEditable(false, true, false);
	clientRxPortLabel.setEditable(false, true, false);
	clientTxIpLabel.setText("127.0.0.1", dontSendNotification);
	clientTxPortLabel.setText("6000", dontSendNotification);
	clientRxPortLabel.setText("9001", dontSendNotification);
	clientTxIpLabel.setColour(Label::outlineColourId,Colours::black);
	clientTxPortLabel.setColour(Label::outlineColourId, Colours::black);
	clientRxPortLabel.setColour(Label::outlineColourId, Colours::black);
	clientTxIpLabel.setJustificationType(Justification::centred);
	clientTxPortLabel.setJustificationType(Justification::centred);
	clientRxPortLabel.setJustificationType(Justification::centred);
	addAndMakeVisible(clientTxIpLabel);
	addAndMakeVisible(clientTxPortLabel);
	addAndMakeVisible(clientRxPortLabel);
    
    // OSC sender and receiver connect
	String clientIp = clientTxIpLabel.getText();
	int clientSendToPort = clientTxPortLabel.getText().getIntValue();
	int clientReceiveAtPort = clientRxPortLabel.getText().getIntValue();

    remoteInterfaceTxRx.connectSender(clientIp, clientSendToPort);
    remoteInterfaceTxRx.connectReceiver(clientReceiveAtPort);
    remoteInterfaceTxRx.addListener(this);

	addAndMakeVisible(&openAudioDeviceManager);
	openAudioDeviceManager.setButtonText("Audio device setup");
	openAudioDeviceManager.addListener(this);
    
	// optional test interfeace load buttons
	addAndMakeVisible(&loadOSCTestButton);
	loadOSCTestButton.setButtonText("OSC Communication Test");
	loadOSCTestButton.addListener(this);

	addAndMakeVisible(&loadMushraBtn);
	loadMushraBtn.setButtonText("MUSHRA Test");
	loadMushraBtn.addListener(this);

	addAndMakeVisible(&loadLocTestBtn);
	loadLocTestBtn.setButtonText("Localisation Test");
	loadLocTestBtn.addListener(this);

	// log window
    addAndMakeVisible(logWindow);
    logWindow.setMultiLine(true);
    logWindow.setReadOnly(true);
    logWindow.setCaretVisible(false);
    logWindow.setScrollbarsShown(true);
}

MainComponent::~MainComponent()
{
	br.deinit();
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	// prepare stimulus player object
	sp.prepareToPlay(samplesPerBlockExpected, sampleRate);
	br.prepareToPlay(samplesPerBlockExpected, sampleRate);

	if(samplesPerBlockExpected > m_maxSamplesPerBlock)
	{
		m_maxSamplesPerBlock = samplesPerBlockExpected;
		processBuffer.setSize(64, samplesPerBlockExpected);
	}
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
	AudioSourceChannelInfo newinfo(processBuffer);

	// pass the buffer into the stimulus player to be filled with required audio
	sp.getNextAudioBlock(newinfo);
	// pass the buffer to the binaural rendering object to replace ambisonic signals with binaural audio
	br.getNextAudioBlock(newinfo);

	AudioBuffer<float>* sourceBuffer = bufferToFill.buffer;

	for (int c = 0; c < sourceBuffer->getNumChannels(); ++c)
	{
		sourceBuffer->copyFrom(c, 0, *newinfo.buffer, c, 0, sourceBuffer->getNumSamples());
	}
}

void MainComponent::releaseResources()
{
	// relese resources taken by stimulus player object
    sp.releaseResources();
	br.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
	// RECTANGULAR OUTLINE
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);
    juce::Rectangle<int> oscRect(250, 10, 330, 150);        // osc status / vr interface status
    juce::Rectangle<int> tstlogicRect(10, 170, 570, 480);   // test logic component
    juce::Rectangle<int> renderRect(590, 405, 800, 385);    // rendering component
    g.drawRect(oscRect, 1);
    g.drawRect(tstlogicRect, 1);
    g.drawRect(renderRect, 1);

	// OSC WINDOW
	g.setColour(Colours::white);

	g.drawText("IP", 310, 10, 50, 25, Justification::centredLeft, true);
	g.drawText("Send to", 410, 10, 50, 25, Justification::centredLeft, true);
	g.drawText("Receive at", 490, 10, 75, 25, Justification::centredLeft, true);
	g.drawText("Client", 260, 35, 50, 25, Justification::centredLeft, true);

    
}

void MainComponent::resized()
{

	as.setCentrePosition(getWidth()/2, getHeight()/2);
    sp.setBounds(590, 10, 800, 385);
	br.setBounds(590, 405, 800, 385);
	openAudioDeviceManager.setBounds(10, 40, 230, 25);

	clientTxIpLabel.setBounds(310, 35, 80, 25);
	clientTxPortLabel.setBounds(410, 35, 60, 25);
	clientRxPortLabel.setBounds(490, 35, 60, 25);

	loadOSCTestButton.setBounds(40, 200, 250, 25);
	loadMushraBtn.setBounds(40, 250, 250, 25);
	loadLocTestBtn.setBounds(40, 300, 250, 25);


    logWindow.setBounds(10, 660, 570, 130);

	// fit OSC test component
	otc.setBounds(10, 170, 570, 480);
    // fit mushra interface
    mc.setBounds(10, 170, 570, 480);
}

void MainComponent::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &openAudioDeviceManager)
	{
		if (as.isVisible())
			as.setVisible(false);
		else
			addAndMakeVisible(as);
	}

	if (buttonThatWasClicked == &loadOSCTestButton)
	{
		// add OSC test component
		addAndMakeVisible(otc);
	}

	if (buttonThatWasClicked == &loadMushraBtn)
	{
		configureMushra();
	}

	if (buttonThatWasClicked == &loadLocTestBtn)
	{

	}


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

	// HEAD TRACKING DATA
	if (message.size() == 3 && message.getAddressPattern() == "/rpy")
	{
		br.setHeadTrackingData(message[0].getFloat32(), message[1].getFloat32(), message[2].getFloat32());
	}

	if (message.size() == 1 && message.getAddressPattern() == "/sofaload" && message[0].isString())
	{
		File sourcePath = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("SALTE");

		if (sourcePath.exists())
		{
			String filename = message[0].getString();
			sourcePath = sourcePath.getChildFile(filename);

			if (sourcePath.existsAsFile())
			{
				br.loadSofaFile(sourcePath);
			}
			else
			{
				Logger::outputDebugString("SOFA file could not be located, please check that it exists");
			}
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
	else if (source == &br)
	{
		logWindowMessage += br.m_currentLogMessage;
		br.m_currentLogMessage.clear();
	}
    
    logWindow.setText(logWindowMessage);
    logWindow.moveCaretToEnd();
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
