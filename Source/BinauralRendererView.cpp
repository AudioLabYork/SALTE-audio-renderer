#include "BinauralRendererView.h"
#include "ROM.h"

BinauralRendererView::BinauralRendererView()
	: m_renderer(nullptr)
{

}

void BinauralRendererView::init(BinauralRenderer* renderer)
{
	m_renderer = renderer;

	m_enableRenderer.setButtonText("Enable Binaural Renderer");
	m_enableRenderer.setToggleState(m_renderer->isRendererEnabled(), dontSendNotification);
	m_enableRenderer.addListener(this);
	addAndMakeVisible(m_enableRenderer);

	m_ambixFileLabel.setText("AmbiX Config File:", dontSendNotification);
	addAndMakeVisible(m_ambixFileLabel);

	m_ambixFileBrowse.setButtonText("Select file...");
	m_ambixFileBrowse.addListener(this);
	addAndMakeVisible(m_ambixFileBrowse);

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
	g.drawRect(getLocalBounds(), 1);

	const int border = 5;
	const int headSize = getHeight() - 2 * border;

	juce::Rectangle<int>headbox(getWidth() - headSize - border, border, headSize, headSize);

	g.drawRect(headbox, 1);
}

void BinauralRendererView::resized()
{
	m_ambixFileLabel.setBounds(10, 10, 390, 25);
	m_ambixFileBrowse.setBounds(400, 10, 80, 25);

	m_sofaFileLabel.setBounds(10, 40, 390, 25);
	m_sofaFileBrowse.setBounds(400, 40, 80, 25);

	m_enableRenderer.setBounds(10, 70, 200, 25);
	m_enableDualBand.setBounds(10, 100, 200, 25);
	m_enableRotation.setBounds(10, 130, 200, 25);
	m_enableMirrorView.setBounds(10, 160, 200, 25);

	const int border = 5;
	const int headSize = getHeight() - 2 * border;

	m_binauralHeadView.setBounds(getWidth() - headSize - border, border, headSize, headSize);

	m_rollLabel.setBounds(400, 175, 150, 20);
	m_pitchLabel.setBounds(400, 195, 150, 20);
	m_yawLabel.setBounds(400, 215, 150, 20);
}

void BinauralRendererView::buttonClicked(Button* buttonClicked)
{
	if (buttonClicked == &m_ambixFileBrowse)
	{
		browseForAmbixConfigFile();
	}
	else if (buttonClicked == &m_sofaFileBrowse)
	{
		browseForSofaFile();
	}
	else if (buttonClicked == &m_enableDualBand)
	{
		m_renderer->enableDualBand(m_enableDualBand.getToggleState());
	}
	else if (buttonClicked == &m_enableRotation)
	{
		m_renderer->enableRotation(m_enableRotation.getToggleState());
	}
	else if (buttonClicked == &m_enableRenderer)
	{
		m_renderer->enableRenderer(m_enableRenderer.getToggleState());
	}
}

void BinauralRendererView::ambixFileLoaded(const File& file)
{
	m_ambixFileLabel.setText("AmbiX Config File: " + file.getFileName(), NotificationType::dontSendNotification);
}

void BinauralRendererView::sofaFileLoaded(const File& file)
{
	m_sofaFileLabel.setText("SOFA File: " + file.getFileName(), NotificationType::dontSendNotification);
}

void BinauralRendererView::setTestInProgress(bool inProgress)
{
	if (inProgress)
	{
		m_sofaFileBrowse.setEnabled(false);
		m_ambixFileBrowse.setEnabled(false);
	}
	else
	{
		m_sofaFileBrowse.setEnabled(true);
		m_ambixFileBrowse.setEnabled(true);
	}
}

void BinauralRendererView::browseForAmbixConfigFile()
{
#if JUCE_MODAL_LOOPS_PERMITTED
	FileChooser fc("Select Ambix Config file to open...",
		File::getCurrentWorkingDirectory(),
		"*.config",
		true);

	if (fc.browseForFileToOpen())
	{
		File chosenFile = fc.getResult();

		m_renderer->initialiseFromAmbix(chosenFile);

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

		m_renderer->loadHRIRsFromSofaFile(chosenFile);
	}
#endif
}

void BinauralRendererView::timerCallback()
{
	if (m_enableRotation.getToggleState())
	{
		m_rollLabel.setText("R: " + String(m_renderer->getRoll(), 1) + " deg", dontSendNotification);
		m_pitchLabel.setText("P: " + String(m_renderer->getPitch(), 1) + " deg", dontSendNotification);
		m_yawLabel.setText("Y: " + String(m_renderer->getYaw(), 1) + " deg", dontSendNotification);

		if (m_enableMirrorView.getToggleState())
			m_binauralHeadView.setHeadOrientation(m_renderer->getRoll(), m_renderer->getPitch(), -m_renderer->getYaw());
		else
			m_binauralHeadView.setHeadOrientation(m_renderer->getRoll(), m_renderer->getPitch(), m_renderer->getYaw() + 180);
	}
}
