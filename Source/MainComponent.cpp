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
	br.setUseSHDConv(true);
	
	brv.init(&br);
	brv.addChangeListener(this);
	addAndMakeVisible(brv);

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
	clientRxPortLabel.setText("9000", dontSendNotification);
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

	addAndMakeVisible(&loadTS126259TestBtn);
	loadTS126259TestBtn.setButtonText("TS 26.259");
	loadTS126259TestBtn.addListener(this);

	// load settings file if available
	String filePath = File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile).getParentDirectory().getFullPathName();
	settingsFile = File(filePath + "/" + "AudioSettings.conf");
	if (settingsFile.existsAsFile())
	{
		loadSettings();
	}

	// log window
    addAndMakeVisible(logWindow);
    logWindow.setMultiLine(true);
    logWindow.setReadOnly(true);
    logWindow.setCaretVisible(false);
    logWindow.setScrollbarsShown(true);
}

MainComponent::~MainComponent()
{
	saveSettings();
	br.deinit();
	brv.deinit();
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
	if (sp.getNumberOfChannels() > 3) br.getNextAudioBlock(newinfo);

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
    juce::Rectangle<int> oscRect(250, 10, 400, 150);        // osc status / vr interface status
    juce::Rectangle<int> tstlogicRect(10, 170, 640, 480);   // test logic component
    juce::Rectangle<int> renderRect(660, 405, 730, 385);    // rendering component
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
    sp.setBounds(660, 10, 730, 385);
	brv.setBounds(660, 405, 730, 385);
	openAudioDeviceManager.setBounds(10, 10, 230, 25);

	clientTxIpLabel.setBounds(310, 35, 80, 25);
	clientTxPortLabel.setBounds(410, 35, 60, 25);
	clientRxPortLabel.setBounds(490, 35, 60, 25);

	loadOSCTestButton.setBounds(40, 200, 250, 25);
	loadMushraBtn.setBounds(40, 250, 250, 25);
	loadLocTestBtn.setBounds(40, 300, 250, 25);
	loadTS126259TestBtn.setBounds(40, 350, 250, 25);


    logWindow.setBounds(10, 660, 640, 130);

	// fit test components
	otc.setBounds(10, 170, 640, 480);
	tsc.setBounds(10, 170, 640, 480);
    mc.setBounds(10, 170, 640, 480);
}

void MainComponent::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &openAudioDeviceManager)
	{
			addAndMakeVisible(as);
	}

	else if (buttonThatWasClicked == &loadOSCTestButton)
	{
		// add OSC test component
		addAndMakeVisible(otc);
	}

	else if (buttonThatWasClicked == &loadMushraBtn)
	{
		mc.createGui();
		addAndMakeVisible(mc);
	}

	else if (buttonThatWasClicked == &loadLocTestBtn)
	{

	}

	else if (buttonThatWasClicked == &loadTS126259TestBtn)
	{
		// pass the player into the test component so that it can use it for triggering play, pause stop etc
		// pass the renderer as well, so we can set the ambisonic order and hrtfs
		tsc.init(&sp, &brv);
		addAndMakeVisible(tsc);
	}

    repaint();
}

void MainComponent::loadSettings()
{
	XmlDocument asxmldoc(settingsFile);
	std::unique_ptr<XmlElement> audioDeviceSettings (asxmldoc.getDocumentElement());
	deviceManager.initialise(0, 2, audioDeviceSettings.get(), true);
}

void MainComponent::saveSettings()
{
	std::unique_ptr<XmlElement> audioDeviceSettings(deviceManager.createStateXml());
	if (audioDeviceSettings.get())
	{
		audioDeviceSettings->writeTo(settingsFile);
	}
}

// LOG WINDOW
void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
    if(source == &sp)
    {
        logWindowMessage += sp.currentMessage;
        sp.currentMessage.clear();
    }
	else if (source == &brv)
	{
		logWindowMessage += brv.m_currentLogMessage;
		brv.m_currentLogMessage.clear();
	}
    
    logWindow.setText(logWindowMessage);
    logWindow.moveCaretToEnd();
}
