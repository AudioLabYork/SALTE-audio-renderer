#include "BinauralRendererView.h"
#include "ROM.h"

BinauralRendererView::BinauralRendererView()
	: m_renderer(nullptr)
{
	
}

void BinauralRendererView::init(BinauralRenderer* renderer)
{
	m_renderer = renderer;
	
	startTimer(50);

	m_ambixFileBrowse.setButtonText("Select Ambix Config file...");
	m_ambixFileBrowse.addListener(this);
	addAndMakeVisible(m_ambixFileBrowse);

	m_useSofa.setButtonText("Should use SOFA file");
	m_useSofa.addListener(this);
	addAndMakeVisible(m_useSofa);

	m_orderSelect.addItemList(orderChoices, 1);
	m_orderSelect.setSelectedItemIndex(0);
	m_orderSelect.addListener(this);
	addAndMakeVisible(m_orderSelect);

	m_sofaFileBrowse.setButtonText("Select SOFA file...");
	m_sofaFileBrowse.addListener(this);
	addAndMakeVisible(m_sofaFileBrowse);

	m_enableDualBand.setButtonText("Enable dual band");
	m_enableDualBand.setToggleState(false, dontSendNotification);
	m_enableDualBand.addListener(this);
	addAndMakeVisible(m_enableDualBand);

	m_enableRotation.setButtonText("Enable rotation");
	m_enableRotation.setToggleState(true, dontSendNotification);
	m_enableRotation.addListener(this);
	addAndMakeVisible(m_enableRotation);

	m_rollLabel.setText("Roll: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(m_rollLabel);
	m_pitchLabel.setText("Pitch: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(m_pitchLabel);
	m_yawLabel.setText("Yaw: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(m_yawLabel);

	addAndMakeVisible(m_binauralHeadView);
	m_enableMirrorView.setButtonText("Enable mirror view");
	m_enableMirrorView.setToggleState(true, dontSendNotification);
	m_enableMirrorView.addListener(this);
	if(m_binauralHeadView.isVisible()) addAndMakeVisible(m_enableMirrorView);
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
}

void BinauralRendererView::resized()
{
	m_ambixFileBrowse.setBounds(10, 10, 150, 30);
	m_useSofa.setBounds(10, 45, 150, 30);

	m_orderSelect.setBounds(170, 10, 150, 30);
	m_sofaFileBrowse.setBounds(170, 45, 150, 30);

	m_enableDualBand.setBounds(335, 10, 150, 30);

	m_enableRotation.setBounds(10, 115, 150, 30);
	m_rollLabel.setBounds(10, 145, 150, 20);
	m_pitchLabel.setBounds(10, 165, 150, 20);
	m_yawLabel.setBounds(10, 185, 150, 20);

	const int border = 5;
	const int headSize = getHeight() - 2 * border;
	m_binauralHeadView.setBounds(getWidth() - headSize - border, border, headSize, headSize);
	m_enableMirrorView.setBounds(335, 40, 150, 30);
}

void BinauralRendererView::buttonClicked(Button* buttonClicked)
{
	if (buttonClicked == &m_ambixFileBrowse)
	{
		browseForAmbixConfigFile();
	}
	else if (buttonClicked == &m_useSofa)
	{
		m_sofaFileBrowse.setEnabled(m_useSofa.getToggleState());
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
}

void BinauralRendererView::comboBoxChanged(ComboBox* comboBoxChanged)
{
	if (comboBoxChanged == &m_orderSelect)
	{
		sendMsgToLogWindow("Changed to Ambisonic order " + String(m_orderSelect.getSelectedItemIndex() + 1));

		std::vector<float> decodeMatrix;
		std::vector<float> azi;
		std::vector<float> ele;
		int numChans = 0;

		bool isValidChoice = false;

		switch (m_orderSelect.getSelectedItemIndex() + 1)
		{
		case 0:
			break;
		case 1:
			for (int i = 0; i < 6; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					decodeMatrix.push_back(mtx1order[i][j]);
				}

				azi.push_back(az1order[i]);
				ele.push_back(el1order[i]);
			}

			numChans = 6;

			isValidChoice = true;
			break;
		case 2:
			break;
		case 3:
			for (int i = 0; i < 26; ++i)
			{
				for (int j = 0; j < 16; ++j)
				{
					decodeMatrix.push_back(mtx3order[i][j]);
				}

				azi.push_back(az3order[i]);
				ele.push_back(el3order[i]);
			}

			numChans = 26;

			isValidChoice = true;
			break;
		case 4:
			break;
		case 5:
			for (int i = 0; i < 50; ++i)
			{
				for (int j = 0; j < 36; ++j)
				{
					decodeMatrix.push_back(mtx5order[i][j]);
				}

				azi.push_back(az5order[i]);
				ele.push_back(el5order[i]);
			}

			numChans = 50;

			isValidChoice = true;
			break;
		case 6:
			break;
		case 7:
			break;
		default:
			break;
		}

		if (isValidChoice)
		{
			m_renderer->setDecodingMatrix(decodeMatrix);
			m_renderer->setOrder(m_orderSelect.getSelectedItemIndex() + 1);
			m_renderer->setVirtualLoudspeakers(azi, ele, numChans);
			m_renderer->updateMatrices();

			if (m_useSofa.getToggleState())
			{
				// use from sofa file that is currently selected
				File sofaFile(m_sofaFilePath);

				// check the file exists, otherwise just use the standard HRTFs
				if (sofaFile.existsAsFile())
					BinauralRenderer::loadHRIRsFromSofaFile(sofaFile, m_renderer);
				else
					loadStandardHRTF();
			}
			else
			{
				// use from standard HRTFs
				loadStandardHRTF();
			}
		}
	}
}

void BinauralRendererView::setTestInProgress(bool inProgress)
{
	if (inProgress)
	{
		m_sofaFileBrowse.setEnabled(false);
		m_ambixFileBrowse.setEnabled(false);
		m_orderSelect.setEnabled(false);
	}
	else
	{
		m_sofaFileBrowse.setEnabled(true);
		m_ambixFileBrowse.setEnabled(true);
		m_orderSelect.setEnabled(true);
	}
}

void BinauralRendererView::changeComboBox(int order)
{
	m_orderSelect.setSelectedItemIndex(order - 1);
}

void BinauralRendererView::sendMsgToLogWindow(const String& message)
{
	m_currentLogMessage += message + "\n";
	sendChangeMessage();
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

		if (BinauralRenderer::initialiseFromAmbix(chosenFile, m_renderer))
		{
			sendMsgToLogWindow("Successfully loaded AmbiX file: " + String(chosenFile.getFileName()));
		}
		else
		{
			sendMsgToLogWindow("Failed to load AmbiX file: " + String(chosenFile.getFileName()));
		}
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

		if (BinauralRenderer::loadHRIRsFromSofaFile(chosenFile, m_renderer))
		{
			sendMsgToLogWindow("Successfully loaded SOFA file: " + String(chosenFile.getFileName()));
		}
		else
		{
			sendMsgToLogWindow("Failed to load SOFA file: " + String(chosenFile.getFileName()));
		}
	}
#endif
}

void BinauralRendererView::loadStandardHRTF()
{
	std::vector<float> azi;
	std::vector<float> ele;
	int chans = 0;

	m_renderer->getVirtualLoudspeakers(azi, ele, chans);

	m_renderer->clearHRIR();

	String filename;
	File sourcePath = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("SALTE").getChildFile("48K_24bit");

	if (sourcePath.exists())
	{
		for (int i = 0; i < chans; ++i)
		{
			filename = "azi_" + String(azi[i], 1).replaceCharacter('.', ',') + "_ele_" + String(ele[i], 1).replaceCharacter('.', ',') + ".wav";

			File hrirFile(sourcePath.getChildFile(filename));

			if (hrirFile.existsAsFile())
			{
				AudioFormatManager formatManager;
				formatManager.registerBasicFormats();
				std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(hrirFile));

				AudioBuffer<float> inputBuffer(reader->numChannels, static_cast<int>(reader->lengthInSamples));

				reader->read(&inputBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

				m_renderer->addHRIR(inputBuffer);
				sendMsgToLogWindow("Adding HRIR for azi:" + String(azi[i]) + ", ele: " + String(ele[i]));
			}
			else
			{
				sendMsgToLogWindow("Could not find HRIR for azi:" + String(azi[i]) + ", ele: " + String(ele[i]));
			}
		}

		m_renderer->uploadHRIRsToEngine();

		sendMsgToLogWindow("Standard HRIRs were loaded");
	}
	else
	{
		sendMsgToLogWindow("Standard HRIRs folder could not be located");
	}
}

void BinauralRendererView::timerCallback()
{
	if (m_enableRotation.getToggleState())
	{
		m_rollLabel.setText("Roll: " + String(m_renderer->getRoll(), 2) + " deg", dontSendNotification);
		m_pitchLabel.setText("Pitch: " + String(m_renderer->getPitch(), 2) + " deg", dontSendNotification);
		m_yawLabel.setText("Yaw: " + String(m_renderer->getYaw(), 2) + " deg", dontSendNotification);
		m_orderSelect.setSelectedItemIndex(m_renderer->getOrder() - 1, dontSendNotification);

		if(m_enableMirrorView.getToggleState())
			m_binauralHeadView.setHeadOrientation(m_renderer->getRoll(), m_renderer->getPitch(), -m_renderer->getYaw());
		else
			m_binauralHeadView.setHeadOrientation(m_renderer->getRoll(), m_renderer->getPitch(), m_renderer->getYaw() + 180);
	}
}
