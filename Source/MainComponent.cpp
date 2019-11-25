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
	m_binauralRenderer.init();
	oscTxRx.addListener(&m_binauralRenderer);
	m_binauralRenderer.addChangeListener(this);

	m_binauralRendererView.init(&m_binauralRenderer);
	m_binauralRendererView.addChangeListener(this);
	addAndMakeVisible(m_binauralRendererView);

	// initialize headphone compensation
	addAndMakeVisible(m_headphoneCompensation);

	// set size of the main app window
	setSize(1400, 800);

	// add logo
	Image logo = ImageFileFormat::loadFrom(BinaryData::logo_180px_png, BinaryData::logo_180px_pngSize);
	if (logo.isValid()) imageComponent.setImage(logo);
	addAndMakeVisible(&imageComponent);

    //// set number of output channels to 2 (binaural rendering case)
    //setAudioChannels (0, 2);

	// set number of output channels to 50 (rendering using loudspeaker rig)
	setAudioChannels(0, 50);

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

	mc.init(&m_stimulusPlayer, &m_binauralRenderer);
	mc.addListener(this);
	mc.addChangeListener(this);
	addChildComponent(mc);

	// localisation component temporarily on top of the session form and mixed methods
	m_localisationComponent.init(&m_stimulusPlayer, &m_binauralRenderer);
	m_localisationComponent.addChangeListener(this);
	addAndMakeVisible(m_localisationComponent);

	// log window
    logWindow.setMultiLine(true);
    logWindow.setReadOnly(true);
    logWindow.setCaretVisible(false);
    logWindow.setScrollbarsShown(true);
	addAndMakeVisible(logWindow);

	showMixedComp.setButtonText("Mixed Methods");
	showMixedComp.addListener(this);
	addAndMakeVisible(showMixedComp);

	showLocComp.setButtonText("Localisation");
	showLocComp.addListener(this);
	addAndMakeVisible(showLocComp);

	showTestInterface.setButtonText("Show test interface");
	showTestInterface.setClickingTogglesState(true);
	showTestInterface.addListener(this);
	addAndMakeVisible(showTestInterface);

	LookAndFeel& lookAndFeel = getLookAndFeel();
	Colour bckgnd = Colour(25, 50, 77);
	lookAndFeel.setColour(ResizableWindow::backgroundColourId, bckgnd);
	lookAndFeel.setColour(ComboBox::backgroundColourId, bckgnd.darker());
	lookAndFeel.setColour(TextEditor::backgroundColourId, bckgnd.darker());
	lookAndFeel.setColour(Slider::backgroundColourId, bckgnd.darker());
	lookAndFeel.setColour(TextButton::buttonColourId, Colour(12, 25, 39));
	lookAndFeel.setColour(Slider::trackColourId, Colour(12, 25, 39));
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
	m_headphoneCompensation.prepareToPlay(samplesPerBlockExpected, sampleRate);

	if(samplesPerBlockExpected != m_maxSamplesPerBlock)
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
	m_binauralRenderer.processBlock(*newinfo.buffer);
	m_headphoneCompensation.processBlock(*newinfo.buffer);

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
	m_headphoneCompensation.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

	// RECTANGULAR OUTLINE
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

	g.setColour(getLookAndFeel().findColour(Label::textColourId));
	g.drawMultiLineText(String("Build date and time:\n" + String(__DATE__) + " " + String(__TIME__)), 10, 140, 150, Justification::left);
	
	g.setFont(18.0f);
	g.drawMultiLineText("Spatial\nAudio\nListening\nTest\nEnvironment", 120, 33, 120, Justification::left, 1.2f);

    
	if (showOnlyTestInterface)
	{

	}
	else
	{
		juce::Rectangle<int> oscRect(250, 10, 400, 150);        // osc status / vr interface status

		g.setColour(Colours::black);
		g.drawRect(oscRect, 1);


		// OSC WINDOW
		g.setColour(getLookAndFeel().findColour(Label::textColourId));
		g.setFont(14.0f);
		g.drawText("IP", 310, 10, 50, 25, Justification::centredLeft, true);
		g.drawText("Send to", 410, 10, 50, 25, Justification::centredLeft, true);
		g.drawText("Receive at", 490, 10, 75, 25, Justification::centredLeft, true);
		g.drawText("Client", 260, 35, 50, 25, Justification::centredLeft, true);
	}
}

void MainComponent::resized()
{
	imageComponent.setBounds(20, 20, 90, 90);

	m_testSessionForm.setBounds(10, 170, 640, 480);
	mc.setBounds(10, 170, 640, 480);
	m_localisationComponent.setBounds(10, 170, 640, 480);

	if (showOnlyTestInterface)
	{
		m_stimulusPlayer.setBounds(660, 170, 730, 330);
		showTestInterface.setBounds(getWidth() - 20, getHeight()-20, 10, 10);
	}
	else
	{
		m_audioSetup.setCentrePosition(getWidth() / 2, getHeight() / 2);
		m_stimulusPlayer.setBounds(660, 10, 730, 330);
		m_binauralRendererView.setBounds(660, 350, 730, 245);
		m_headphoneCompensation.setBounds(660, 605, 730, 185);

		connectOscButton.setBounds(560, 20, 80, 40);
		openAudioDeviceManager.setBounds(310, 70, 240, 25);

		clientTxIpLabel.setBounds(310, 35, 80, 25);
		clientTxPortLabel.setBounds(410, 35, 60, 25);
		clientRxPortLabel.setBounds(490, 35, 60, 25);

		logWindow.setBounds(10, 660, 640, 130);

		showMixedComp.setBounds(310, 105, 115, 25);
		showLocComp.setBounds(435, 105, 115, 25);
		showTestInterface.setBounds(560, 70, 80, 60);
	}
}

void MainComponent::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &openAudioDeviceManager)
	{
		addAndMakeVisible(m_audioSetup);
		m_audioSetup.m_shouldBeVisible = true;
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
	else if (buttonThatWasClicked == &showMixedComp)
	{
		m_testSessionForm.setVisible(true);
		m_localisationComponent.setVisible(false);
	}
	else if (buttonThatWasClicked == &showLocComp)
	{
		m_testSessionForm.setVisible(false);
		m_localisationComponent.setVisible(true);
	}
	else if (buttonThatWasClicked == &showTestInterface)
	{
		bool show = showTestInterface.getToggleState();
		showOnlyTestInterface = show;
		
		m_stimulusPlayer.setShowTest(show);
		if(m_audioSetup.m_shouldBeVisible) m_audioSetup.setVisible(!show);
		m_binauralRendererView.setVisible(!show);
		m_headphoneCompensation.setVisible(!show);
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
	mc.loadTestSession(&m_testSession, &oscTxRx);
	mc.setVisible(true);
	m_binauralRendererView.setTestInProgress(true);
}

void MainComponent::testCompleted()
{
	m_testSessionForm.reset();
	m_testSessionForm.setVisible(true);
	m_binauralRendererView.setTestInProgress(false);
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
	String timeStamp = Time::getCurrentTime().formatted("%H:%M:%S") + ": ";

    if(source == &m_stimulusPlayer)
    {
		if (m_stimulusPlayer.currentMessage != "")
		{
			logWindowMessage += timeStamp + m_stimulusPlayer.currentMessage;
			m_stimulusPlayer.currentMessage.clear();
		}
    }
	else if (source == &m_binauralRenderer)
	{
		if (m_binauralRenderer.m_currentLogMessage != "")
		{
			logWindowMessage += timeStamp + m_binauralRenderer.m_currentLogMessage;
			m_binauralRenderer.m_currentLogMessage.clear();
		}
	}
	else if (source == &m_binauralRendererView)
	{
		if (m_binauralRendererView.m_currentLogMessage != "")
		{
			logWindowMessage += timeStamp + m_binauralRendererView.m_currentLogMessage;
			m_binauralRendererView.m_currentLogMessage.clear();
		}
	}
	else if (source == &mc)
	{
		if (mc.currentMessage != "")
		{
			logWindowMessage += timeStamp + mc.currentMessage;
			mc.currentMessage.clear();
		}
	}

    logWindow.setText(logWindowMessage);
    logWindow.moveCaretToEnd();
}
