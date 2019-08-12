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
	m_ambixFileBrowse.setEnabled(false);
	addAndMakeVisible(&m_ambixFileBrowse);

	m_useSofa.setButtonText("Should use SOFA file");
	m_useSofa.addListener(this);
	addAndMakeVisible(&m_useSofa);

	m_orderSelect.addItemList(orderChoices, 1);
	m_orderSelect.setSelectedItemIndex(0);
	m_orderSelect.addListener(this);
	addAndMakeVisible(&m_orderSelect);

	m_sofaFileBrowse.setButtonText("Select SOFA file...");
	m_sofaFileBrowse.setEnabled(false);
	m_sofaFileBrowse.addListener(this);
	addAndMakeVisible(&m_sofaFileBrowse);

	m_enableRotation.setButtonText("Enable rotation");
	m_enableRotation.setToggleState(true, dontSendNotification);
	addAndMakeVisible(&m_enableRotation);

	m_rollLabel.setText("Roll: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_rollLabel);
	m_pitchLabel.setText("Pitch: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_pitchLabel);
	m_yawLabel.setText("Yaw: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_yawLabel);
}

void BinauralRendererView::deinit()
{
	stopTimer();
}

void BinauralRendererView::paint(Graphics& /*g*/)
{
}

void BinauralRendererView::resized()
{
	m_ambixFileBrowse.setBounds(10, 10, 150, 30);
	m_useSofa.setBounds(10, 45, 150, 30);

	m_orderSelect.setBounds(170, 10, 150, 30);
	m_sofaFileBrowse.setBounds(170, 45, 150, 30);

	m_enableRotation.setBounds(10, 115, 150, 30);
	m_rollLabel.setBounds(10, 145, 150, 20);
	m_pitchLabel.setBounds(10, 165, 150, 20);
	m_yawLabel.setBounds(10, 185, 150, 20);
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
			m_renderer->setLoudspeakerChannels(azi, ele, numChans);
			m_renderer->updateMatrices();

			if (m_useSofa.getToggleState())
			{
				// use from sofa file that is currently selected
				File sofaFile(m_sofaFilePath);

				// check the file exists, otherwise just use the standard HRTFs
				if (sofaFile.existsAsFile())
					loadSofaFile(sofaFile);
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
		loadAmbixConfigFile(chosenFile);
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
		loadSofaFile(chosenFile);
	}
#endif
}

void BinauralRendererView::loadAmbixConfigFile(const File& file)
{
	FileInputStream fis(file);

	StringArray lines;

	file.readLines(lines);

	while (!fis.isExhausted())
	{
		String line = fis.readNextLine();

		if (line.contains("#GLOBAL"))
		{
			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
					break;

				if (line.startsWithIgnoreCase("/debug_msg"))
				{
					String res = line.fromFirstOccurrenceOf("/debug_msg ", false, true);
					Logger::outputDebugString(res);
				}
				else if (line.startsWithIgnoreCase("/dec_mat_gain"))
				{
					String res = line.fromFirstOccurrenceOf("/dec_mat_gain ", false, true);
					Logger::outputDebugString(res);
				}
				else if (line.startsWithIgnoreCase("/coeff_scale"))
				{
					String res = line.fromFirstOccurrenceOf("/coeff_scale ", false, true);
					Logger::outputDebugString(res);
				}
				else if (line.startsWithIgnoreCase("/coeff_seq"))
				{
					String res = line.fromFirstOccurrenceOf("/coeff_seq ", false, true);
					Logger::outputDebugString(res);
				}
				else if (line.startsWithIgnoreCase("/flip"))
				{
					String res = line.fromFirstOccurrenceOf("/flip ", false, true);
					Logger::outputDebugString(res);
				}
				else if (line.startsWithIgnoreCase("/flop"))
				{
					String res = line.fromFirstOccurrenceOf("/flop ", false, true);
					Logger::outputDebugString(res);
				}
				else if (line.startsWithIgnoreCase("/flap"))
				{
					String res = line.fromFirstOccurrenceOf("/flap ", false, true);
					Logger::outputDebugString(res);
				}
				else if (line.startsWithIgnoreCase("/global_hrtf_gain"))
				{
					String res = line.fromFirstOccurrenceOf("/global_hrtf_gain ", false, true);
					Logger::outputDebugString(res);
				}
				else if (line.startsWithIgnoreCase("/invert_condon_shortley"))
				{
					String res = line.fromFirstOccurrenceOf("/invert_condon_shortley ", false, true);
					Logger::outputDebugString(res);
				}
			}
		}
		else if (line.contains("#HRTF"))
		{
			m_renderer->clearLoudspeakerChannels();
			m_renderer->clearHRIR();

			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
					break;

				String path = file.getParentDirectory().getFullPathName();

				File hrirFile(path + File::getSeparatorString() + line);

				if (hrirFile.existsAsFile())
				{
					AudioFormatManager formatManager;
					formatManager.registerBasicFormats();
					std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(hrirFile));

					AudioBuffer<float> inputBuffer(reader->numChannels, static_cast<int>(reader->lengthInSamples));

					reader->read(&inputBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

					m_renderer->addHRIR(inputBuffer);
				}
			}
		}
		else if (line.contains("#DECODERMATRIX"))
		{
			std::vector<float> decodeMatrix;

			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
				{
					m_renderer->setDecodingMatrix(decodeMatrix);
					break;
				}

				juce::String::CharPointerType charptr = line.getCharPointer();

				while (charptr != charptr.findTerminatingNull())
				{
					float nextval = static_cast<float>(CharacterFunctions::readDoubleValue(charptr));
					decodeMatrix.push_back(nextval);
				}
			}
		}
	}

	m_renderer->updateMatrices();
	m_renderer->preprocessHRIRs();
	m_renderer->uploadHRIRsToEngine();

	sendMsgToLogWindow("Ambix Config file " + file.getFileName() + " was loaded");
}

void BinauralRendererView::loadSofaFile(const File& file)
{
	m_sofaFilePath = file.getFullPathName();

	SOFAReader reader(m_sofaFilePath.toStdString());

	std::vector<float> HRIRData;
	std::size_t channels = reader.getNumImpulseChannels();
	std::size_t samples = reader.getNumImpulseSamples();

	std::vector<float> azi;
	std::vector<float> ele;
	int chans = 0;

	m_renderer->getLoudspeakerChannels(azi, ele, chans);
	
	m_renderer->clearHRIR();

	for (int i = 0; i < chans; ++i)
	{
		if (reader.getResponseForSpeakerPosition(HRIRData, azi[i], ele[i]))
		{
			AudioBuffer<float> inputBuffer(static_cast<int>(channels), static_cast<int>(samples));
	
			for (int c = 0; c < channels; ++c)
				inputBuffer.copyFrom(c, 0, HRIRData.data(), static_cast<int>(samples));

			m_renderer->addHRIR(inputBuffer);
			sendMsgToLogWindow("Adding HRIR for azi:" + String(azi[i]) + ", ele: " + String(ele[i]));
		}
	}

	m_renderer->preprocessHRIRs();
	m_renderer->uploadHRIRsToEngine();

	sendMsgToLogWindow("SOFA file " + file.getFileName() + " was loaded");
}

void BinauralRendererView::loadStandardHRTF()
{
	std::vector<float> azi;
	std::vector<float> ele;
	int chans = 0;

	m_renderer->getLoudspeakerChannels(azi, ele, chans);

	m_renderer->clearHRIR();

	String filename;
	File sourcePath = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("SALTE").getChildFile("48K_24bit");

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
	}

	m_renderer->uploadHRIRsToEngine();

	sendMsgToLogWindow("Standard HRIRs were loaded");
}

void BinauralRendererView::timerCallback()
{
	if (m_enableRotation.getToggleState())
	{
		m_rollLabel.setText("Roll: " + String(m_renderer->m_roll,2) + " deg", dontSendNotification);
		m_pitchLabel.setText("Pitch: " + String(m_renderer->m_pitch, 2) + " deg", dontSendNotification);
		m_yawLabel.setText("Yaw: " + String(m_renderer->m_yaw, 2) + " deg", dontSendNotification);
	}
}
