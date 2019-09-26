#include "MainComponent.h"

MainComponent::MainComponent()
	: m_audioSetup(deviceManager)
	, m_maxSamplesPerBlock(0)
	, showOnlyTestInterface(false)
{
    // add and make visible the stimulus player object
    addAndMakeVisible(m_stimulusPlayer);
    m_stimulusPlayer.addChangeListener(this);

	// setup binaural renderer, pass the osc transceiver
	m_binauralRenderer.init(&oscTxRx);

	m_binauralRendererView.init(&m_binauralRenderer);
	m_binauralRendererView.addChangeListener(this);
	addAndMakeVisible(m_binauralRendererView);

	File sourcePath(File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("SALTE"));
	Result res = sourcePath.createDirectory();

	if (res.wasOk())
		Logger::outputDebugString("application directory created successfully");

	// set size of the main app window
	setSize(1400, 800);

	// add logo
	Image logo = ImageFileFormat::loadFrom(BinaryData::logo_180px_png, BinaryData::logo_180px_pngSize);
	if (logo.isValid()) imageComponent.setImage(logo);
	addAndMakeVisible(&imageComponent);

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
	
	connectOscButton.setButtonText("Connect OSC");
	connectOscButton.addListener(this);
	connectOscButton.triggerClick(); // connect on startup
	addAndMakeVisible(&connectOscButton);

	openAudioDeviceManager.setButtonText("Audio device setup");
	openAudioDeviceManager.addListener(this);
	addAndMakeVisible(&openAudioDeviceManager);

	// load settings file if available
	String filePath = File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile).getParentDirectory().getFullPathName();
	audioSettingsFile = File(filePath + "/" + "SALTEAudioSettings.conf");
	
	if (audioSettingsFile.existsAsFile())
		loadSettings();
	
	m_testSessionForm.init(&m_testSession);
	m_testSessionForm.addListener(this);
	addAndMakeVisible(m_testSessionForm);

	mc.init(&oscTxRx, &m_stimulusPlayer, &m_binauralRenderer);
	mc.addListener(this);
	mc.addChangeListener(this);
	addChildComponent(mc);

	// log window
    logWindow.setMultiLine(true);
    logWindow.setReadOnly(true);
    logWindow.setCaretVisible(false);
    logWindow.setScrollbarsShown(true);
	addAndMakeVisible(logWindow);

	showTestInterface.setButtonText("Show test interface");
	showTestInterface.setClickingTogglesState(true);
	showTestInterface.addListener(this);
	addAndMakeVisible(showTestInterface);

	//LookAndFeel& lookAndFeel = getLookAndFeel();
	//lookAndFeel.setColour(ResizableWindow::backgroundColourId, Colours::gainsboro);
	//lookAndFeel.setColour(TextButton::buttonColourId, Colours::gainsboro.darker());
	//lookAndFeel.setColour(TextButton::textColourOffId, Colours::black);
	//lookAndFeel.setColour(ComboBox::backgroundColourId, Colours::gainsboro.darker());
	//lookAndFeel.setColour(ComboBox::textColourId, Colours::black);
	//lookAndFeel.setColour(TextEditor::backgroundColourId, Colours::gainsboro.darker());
	//lookAndFeel.setColour(TextEditor::textColourId, Colours::black);
	//lookAndFeel.setColour(Label::textColourId, Colours::black);
	//lookAndFeel.setColour(Slider::backgroundColourId, Colours::gainsboro.darker());
	//lookAndFeel.setColour(Slider::thumbColourId, Colours::white);
}

MainComponent::~MainComponent()
{
	saveSettings();
	oscTxRx.disconnectTxRx();
	m_binauralRenderer.deinit();
	m_binauralRendererView.deinit();
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	// prepare stimulus player object
	m_stimulusPlayer.prepareToPlay(samplesPerBlockExpected, sampleRate);
	m_binauralRenderer.prepareToPlay(samplesPerBlockExpected, sampleRate);

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
	m_stimulusPlayer.getNextAudioBlock(newinfo);

	// pass the buffer to the binaural rendering object to replace ambisonic signals with binaural audio
	m_binauralRenderer.getNextAudioBlock(newinfo);

	AudioBuffer<float>* sourceBuffer = bufferToFill.buffer;

	for (int c = 0; c < sourceBuffer->getNumChannels(); ++c)
	{
		sourceBuffer->copyFrom(c, 0, *newinfo.buffer, c, 0, sourceBuffer->getNumSamples());
	}
}

void MainComponent::releaseResources()
{
	// relese resources taken by stimulus player object
    m_stimulusPlayer.releaseResources();
	m_binauralRenderer.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
	if (showOnlyTestInterface)
	{

	}
	else
	{
		// RECTANGULAR OUTLINE
		g.setColour(Colours::black);
		g.drawRect(getLocalBounds(), 1);
		juce::Rectangle<int> oscRect(250, 10, 400, 150);        // osc status / vr interface status
		juce::Rectangle<int> tstlogicRect(10, 170, 640, 480);   // test logic component
		juce::Rectangle<int> renderRect(660, 405, 730, 245);    // rendering component
		juce::Rectangle<int> testRect(10, 170, 640, 480);    // test component

		g.drawRect(oscRect, 1);
		g.drawRect(tstlogicRect, 1);
		g.drawRect(renderRect, 1);
		g.drawRect(testRect, 1);

		// OSC WINDOW
		g.setColour(getLookAndFeel().findColour(Label::textColourId));

		g.drawText("IP", 310, 10, 50, 25, Justification::centredLeft, true);
		g.drawText("Send to", 410, 10, 50, 25, Justification::centredLeft, true);
		g.drawText("Receive at", 490, 10, 75, 25, Justification::centredLeft, true);
		g.drawText("Client", 260, 35, 50, 25, Justification::centredLeft, true);
	}
}

void MainComponent::resized()
{
	if (showOnlyTestInterface)
	{
		m_stimulusPlayer.setBounds(660, 10, 730, 480);
		m_testSessionForm.setBounds(10, 10, getWidth() - 20, getHeight() - 20);
		mc.setBounds(10, 10, 640, 480);
		showTestInterface.setBounds(getWidth() - 10, getHeight()-10, 10, 10);
	}
	else
	{
		// imageComponent.setBounds(140, 60, 90, 90);
		imageComponent.setBounds(20, 20, 75, 75);

		m_audioSetup.setCentrePosition(getWidth() / 2, getHeight() / 2);
		m_stimulusPlayer.setBounds(660, 10, 730, 385);
		m_binauralRendererView.setBounds(660, 405, 730, 245);
		connectOscButton.setBounds(310, 70, 240, 25);
		openAudioDeviceManager.setBounds(310, 105, 240, 25);

		clientTxIpLabel.setBounds(310, 35, 80, 25);
		clientTxPortLabel.setBounds(410, 35, 60, 25);
		clientRxPortLabel.setBounds(490, 35, 60, 25);

		logWindow.setBounds(10, 660, 640, 130);

		m_testSessionForm.setBounds(10, 170, 640, 480);
		mc.setBounds(10, 170, 640, 480);

		showTestInterface.setBounds(560, 70, 80, 60);
	}
}

void MainComponent::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &openAudioDeviceManager)
	{
		addAndMakeVisible(m_audioSetup);
	}
	else if (buttonThatWasClicked == &connectOscButton)
	{
		if (!oscTxRx.isConnected())
		{
			// OSC sender and receiver connect
			String clientIp = clientTxIpLabel.getText();
			int clientSendToPort = clientTxPortLabel.getText().getIntValue();
			int clientReceiveAtPort = clientRxPortLabel.getText().getIntValue();
			oscTxRx.connectTxRx(clientIp, clientSendToPort, clientReceiveAtPort);
		}
		else
		{
			oscTxRx.disconnectTxRx();
		}
		
		if (oscTxRx.isConnected())
		{
			connectOscButton.setColour(TextButton::buttonColourId, Colours::green);
			connectOscButton.setButtonText("OSC connected");
		}
		else
		{
			connectOscButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
			connectOscButton.setButtonText("Connect OSC");
		}
	}
	else if (buttonThatWasClicked == &showTestInterface)
	{
		bool show = showTestInterface.getToggleState();
		showOnlyTestInterface = show;
		
		m_stimulusPlayer.setShowTest(show);
		m_audioSetup.setVisible(!show);
		m_binauralRendererView.setVisible(!show);
		openAudioDeviceManager.setVisible(!show);
		connectOscButton.setVisible(!show);
		clientTxIpLabel.setVisible(!show);
		clientTxPortLabel.setVisible(!show);
		clientRxPortLabel.setVisible(!show);
		logWindow.setVisible(!show);
		
		resized();
	}

    repaint();
}

void MainComponent::formCompleted()
{
	mc.loadTestSession(&m_testSession);
	mc.setVisible(true);
}

void MainComponent::testCompleted()
{
	m_testSessionForm.reset();
	m_testSessionForm.setVisible(true);
}

void MainComponent::loadSettings()
{
	XmlDocument asxmldoc(audioSettingsFile);
	std::unique_ptr<XmlElement> audioDeviceSettings (asxmldoc.getDocumentElement());
	deviceManager.initialise(0, 2, audioDeviceSettings.get(), true);
}

void MainComponent::saveSettings()
{
	std::unique_ptr<XmlElement> audioDeviceSettings(deviceManager.createStateXml());
	if (audioDeviceSettings.get())
	{
		audioDeviceSettings->writeTo(audioSettingsFile);
	}
}

// LOG WINDOW
void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
    if(source == &m_stimulusPlayer)
    {
        logWindowMessage += m_stimulusPlayer.currentMessage;
        m_stimulusPlayer.currentMessage.clear();
    }
	else if (source == &m_binauralRendererView)
	{
		logWindowMessage += m_binauralRendererView.m_currentLogMessage;
		m_binauralRendererView.m_currentLogMessage.clear();
	}
	else if (source == &mc)
	{
		logWindowMessage += mc.currentMessage;
		mc.currentMessage.clear();
	}

    logWindow.setText(logWindowMessage);
    logWindow.moveCaretToEnd();
}
