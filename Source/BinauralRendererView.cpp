#include "BinauralRendererView.h"

BinauralRendererView::BinauralRendererView()
	: m_binRenderer(nullptr)
	, modeSelectTabs(TabbedButtonBar::Orientation::TabsAtTop)
{

}

void BinauralRendererView::init(LoudspeakerRenderer* lsRenderer, BinauralRenderer* binRenderer)
{
	m_lsRenderer = lsRenderer;
	m_binRenderer = binRenderer;

	Colour bckgnd = Colour(25, 50, 77);
	modeSelectTabs.addTab("Renderer Bypassed", bckgnd, 0);
	modeSelectTabs.addTab("Loudspeaker Rendering", bckgnd, 1);
	modeSelectTabs.addTab("Binaural Rendering", bckgnd, 2);
	modeSelectTabs.setColour(TabbedButtonBar::tabOutlineColourId, Colours::black);
	modeSelectTabs.setColour(TabbedButtonBar::frontOutlineColourId, Colours::black); // looks like this doesn't work
	modeSelectTabs.addChangeListener(this);
	addAndMakeVisible(modeSelectTabs);

	// Loudspeaker Rendering
	m_lsAmbixFileLabel.setText("AmbiX Config File:", dontSendNotification);
	addAndMakeVisible(m_lsAmbixFileLabel);

	m_lsAmbixFileBrowse.setButtonText("Select file...");
	m_lsAmbixFileBrowse.addListener(this);
	addAndMakeVisible(m_lsAmbixFileBrowse);

	// Binaural Rendering
	m_binAmbixFileLabel.setText("AmbiX Config File:", dontSendNotification);
	addAndMakeVisible(m_binAmbixFileLabel);

	m_binAmbixFileBrowse.setButtonText("Select file...");
	m_binAmbixFileBrowse.addListener(this);
	addAndMakeVisible(m_binAmbixFileBrowse);

	m_sofaFileLabel.setText("SOFA File:", dontSendNotification);
	addAndMakeVisible(m_sofaFileLabel);

	m_sofaFileBrowse.setButtonText("Select file...");
	m_sofaFileBrowse.setEnabled(false);
	m_sofaFileBrowse.addListener(this);
	addAndMakeVisible(m_sofaFileBrowse);

	m_enableDualBand.setButtonText("Enable dual band");
	m_enableDualBand.setToggleState(true, dontSendNotification);
	m_enableDualBand.addListener(this);
	addAndMakeVisible(m_enableDualBand);

	m_enableRotation.setButtonText("Enable rotation");
	m_enableRotation.setToggleState(true, dontSendNotification);
	m_enableRotation.addListener(this);
	addAndMakeVisible(m_enableRotation);

	addAndMakeVisible(m_binauralHeadView);
	m_enableMirrorView.setButtonText("Enable mirror view");
	m_enableMirrorView.setToggleState(true, dontSendNotification);
	m_enableMirrorView.addListener(this);

	if (m_binauralHeadView.isVisible())
		addAndMakeVisible(m_enableMirrorView);

	m_rollLabel.setText("R: 0.0 deg", dontSendNotification);
	addAndMakeVisible(m_rollLabel);
	m_pitchLabel.setText("P: 0.0 deg", dontSendNotification);
	addAndMakeVisible(m_pitchLabel);
	m_yawLabel.setText("Y: 0.0 deg", dontSendNotification);
	addAndMakeVisible(m_yawLabel);

	startTimer(50);
	modeSelectTabs.setCurrentTabIndex(0);
	setRendererMode(bypassed);
}

void BinauralRendererView::deinit()
{
	stopTimer();
}

void BinauralRendererView::paint(Graphics& g)
{
	// BACKGROUND
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	// RECTANGULAR OUTLINE
	g.setColour(Colours::black);
	juce::Rectangle<int> area(getLocalBounds().withTrimmedTop(25));
	//g.drawRect(getLocalBounds().withTrimmedTop(25), 1);
	Line<float> line1(	Point<float> (area.getTopLeft().getX(), area.getTopLeft().getY()),
						Point<float> (area.getBottomLeft().getX(), area.getBottomLeft().getY()));
	Line<float> line2(	Point<float>(area.getBottomLeft().getX(), area.getBottomLeft().getY()),
						Point<float>(area.getBottomRight().getX(), area.getBottomRight().getY()));
	Line<float> line3(	Point<float>(area.getBottomRight().getX(), area.getBottomRight().getY()),
						Point<float>(area.getTopRight().getX(), area.getTopRight().getY()));
	g.drawLine(line1, 2.0f);
	g.drawLine(line2, 2.0f);
	g.drawLine(line3, 2.0f);
}

void BinauralRendererView::resized()
{
	const int tabBarHeight = 25;
	modeSelectTabs.setBounds(0, 0, getWidth(), tabBarHeight);

	m_lsAmbixFileLabel.setBounds(10, 35, 390, 25);
	m_lsAmbixFileBrowse.setBounds(400, 35, 80, 25);

	m_binAmbixFileLabel.setBounds(10, 35, 390, 25);
	m_binAmbixFileBrowse.setBounds(400, 35, 80, 25);

	m_sofaFileLabel.setBounds(10, 75, 390, 25);
	m_sofaFileBrowse.setBounds(400, 75, 80, 25);

	m_enableDualBand.setBounds(10, 140, 200, 25);
	m_enableRotation.setBounds(10, 170, 200, 25);
	m_enableMirrorView.setBounds(10, 200, 200, 25);

	const int border = 5;
	const int headSize = getHeight() - 2 * border - tabBarHeight;

	m_binauralHeadView.setBounds(getWidth() - headSize - border, border + tabBarHeight, headSize, headSize);

	m_rollLabel.setBounds(400, 175, 150, 20);
	m_pitchLabel.setBounds(400, 195, 150, 20);
	m_yawLabel.setBounds(400, 215, 150, 20);
}

void BinauralRendererView::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == &modeSelectTabs)
	{
		if (modeSelectTabs.getCurrentTabIndex() == 0) setRendererMode(bypassed);
		else if (modeSelectTabs.getCurrentTabIndex() == 1) setRendererMode(loudspeaker);
		else if (modeSelectTabs.getCurrentTabIndex() == 2) setRendererMode(binaural);
	}
}

void BinauralRendererView::buttonClicked(Button* buttonClicked)
{
	if (buttonClicked == &m_lsAmbixFileBrowse)
	{
		browseForLsAmbixConfigFile();
	}
	else if (buttonClicked == &m_binAmbixFileBrowse)
	{
		browseForBinAmbixConfigFile();
	}
	else if (buttonClicked == &m_sofaFileBrowse)
	{
		browseForSofaFile();
	}
	else if (buttonClicked == &m_enableDualBand)
	{
		m_binRenderer->enableDualBand(m_enableDualBand.getToggleState());
	}
	else if (buttonClicked == &m_enableRotation)
	{
		m_binRenderer->enableRotation(m_enableRotation.getToggleState());
	}
}

void BinauralRendererView::ambixFileLoaded(const File& file)
{
	m_binAmbixFileLabel.setText("AmbiX Config File: " + file.getFileName(), NotificationType::dontSendNotification);
}

void BinauralRendererView::sofaFileLoaded(const File& file)
{
	m_sofaFileLabel.setText("SOFA File: " + file.getFileName(), NotificationType::dontSendNotification);
}

void BinauralRendererView::setRendererMode(RendererModes targetMode)
{
	// hide everything
	m_lsAmbixFileBrowse.setVisible(false);
	m_lsAmbixFileLabel.setVisible(false);
	m_binAmbixFileBrowse.setVisible(false);
	m_binAmbixFileLabel.setVisible(false);
	m_sofaFileBrowse.setVisible(false);
	m_sofaFileLabel.setVisible(false);

	m_enableDualBand.setVisible(false);
	m_enableRotation.setVisible(false);
	
	m_rollLabel.setVisible(false);
	m_pitchLabel.setVisible(false);
	m_yawLabel.setVisible(false);
	
	m_binauralHeadView.setVisible(false);
	m_enableMirrorView.setVisible(false);

	// bypass renderers
	m_binRenderer->enableRenderer(false);

	// turn on what is needed
	switch (targetMode)
	{
	case bypassed:
	{
		break;
	}
	case loudspeaker:
	{
		m_lsAmbixFileBrowse.setVisible(true);
		m_lsAmbixFileLabel.setVisible(true);

		m_lsRenderer->enableRenderer(true);
		break;
	}
	case binaural:
	{
		m_binAmbixFileBrowse.setVisible(true);
		m_binAmbixFileLabel.setVisible(true);
		
		m_sofaFileBrowse.setVisible(true);
		m_sofaFileLabel.setVisible(true);

		m_enableDualBand.setVisible(true);
		m_enableRotation.setVisible(true);
		
		m_rollLabel.setVisible(true);
		m_pitchLabel.setVisible(true);
		m_yawLabel.setVisible(true);

		m_binauralHeadView.setVisible(true);
		m_enableMirrorView.setVisible(true);

		m_binRenderer->enableRenderer(true);
		break;
	}
	default:
		break;
	}
}

void BinauralRendererView::setTestInProgress(bool inProgress)
{
	if (inProgress)
	{
		m_lsAmbixFileBrowse.setEnabled(false);
		m_binAmbixFileBrowse.setEnabled(false);
		m_sofaFileBrowse.setEnabled(false);
	}
	else
	{
		m_lsAmbixFileBrowse.setEnabled(true);
		m_binAmbixFileBrowse.setEnabled(true);
		m_sofaFileBrowse.setEnabled(true);
	}
}

void BinauralRendererView::browseForLsAmbixConfigFile()
{
#if JUCE_MODAL_LOOPS_PERMITTED
	FileChooser fc("Select Ambix Config file to open...",
		File::getCurrentWorkingDirectory(),
		"*.config",
		true);

	if (fc.browseForFileToOpen())
	{
		File chosenFile = fc.getResult();

		//m_binRenderer->initialiseFromAmbix(chosenFile);
		//m_sofaFileBrowse.setEnabled(true);
	}
#endif
}

void BinauralRendererView::browseForBinAmbixConfigFile()
{
#if JUCE_MODAL_LOOPS_PERMITTED
	FileChooser fc("Select Ambix Config file to open...",
		File::getCurrentWorkingDirectory(),
		"*.config",
		true);

	if (fc.browseForFileToOpen())
	{
		File chosenFile = fc.getResult();

		m_binRenderer->initialiseFromAmbix(chosenFile);
		m_sofaFileBrowse.setEnabled(true);
	}
#endif
}

void BinauralRendererView::browseForSofaFile()
{
#if JUCE_MODAL_LOOPS_PERMITTED
	FileChooser fc("Select SOFA file to open...",
		File::getCurrentWorkingDirectory(),
		"*.sofa",
		true);

	if (fc.browseForFileToOpen())
	{
		File chosenFile = fc.getResult();

		m_binRenderer->loadHRIRsFromSofaFile(chosenFile);
	}
#endif
}

void BinauralRendererView::timerCallback()
{
	if (m_enableRotation.getToggleState())
	{
		m_rollLabel.setText("R: " + String(m_binRenderer->getRoll(), 1) + " deg", dontSendNotification);
		m_pitchLabel.setText("P: " + String(m_binRenderer->getPitch(), 1) + " deg", dontSendNotification);
		m_yawLabel.setText("Y: " + String(m_binRenderer->getYaw(), 1) + " deg", dontSendNotification);

		if (m_enableMirrorView.getToggleState())
			m_binauralHeadView.setHeadOrientation(m_binRenderer->getRoll(), m_binRenderer->getPitch(), -m_binRenderer->getYaw());
		else
			m_binauralHeadView.setHeadOrientation(m_binRenderer->getRoll(), m_binRenderer->getPitch(), m_binRenderer->getYaw() + 180);
	}
}
