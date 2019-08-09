#include "BinauralRenderer.h"
#include "ROM.h"

BinauralRenderer::BinauralRenderer()
	: m_order(0)
	, m_numAmbiChans(1)
	, m_numLsChans(0)
	, m_numHrirLoaded(0)
	, m_blockSize(0)
	, m_sampleRate(0.0)
	, m_yaw(0.0f)
	, m_pitch(0.0f)
	, m_roll(0.0f)
	, m_isConfigChanging(false)
	, m_useSHDConv(false)
{
}

void BinauralRenderer::init()
{
	startTimer(100);

	m_ambixFileBrowse.setButtonText("Select Ambix Config file...");
	m_ambixFileBrowse.addListener(this);
	addAndMakeVisible(&m_ambixFileBrowse);

	m_useSofa.setButtonText("Should use SOFA file");
	m_useSofa.addListener(this);
	addAndMakeVisible(&m_useSofa);

	m_orderSelect.addItemList(orderChoices, 1);
	m_orderSelect.setSelectedItemIndex(0, false);
	m_orderSelect.addListener(this);
	addAndMakeVisible(&m_orderSelect);

	m_sofaFileBrowse.setButtonText("Select SOFA file...");
	m_sofaFileBrowse.setEnabled(false);
	m_sofaFileBrowse.addListener(this);
	addAndMakeVisible(&m_sofaFileBrowse);

	m_enableRotation.setButtonText("Enable rotation");
	addAndMakeVisible(&m_enableRotation);

	m_yAxisVal.setText("Roll: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_xAxisVal);
	m_xAxisVal.setText("Pitch: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_yAxisVal);
	m_zAxisVal.setText("Yaw: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_zAxisVal);
}

void BinauralRenderer::reset()
{
	m_order = 0;
	m_numAmbiChans = 1;

	m_azi.clear();
	m_ele.clear();
	m_numLsChans = 0;

	m_blockSize = 0;
	m_sampleRate = 0.0;
}

void BinauralRenderer::deinit()
{
	stopTimer();
}

void BinauralRenderer::setOrder(const int order)
{
	m_order = order;
	m_numAmbiChans = (order + 1) * (order + 1);

	m_hrirShdBuffers.resize(m_numAmbiChans);
}

void BinauralRenderer::setLoudspeakerChannels(std::vector<float>& azimuths, std::vector<float>& elevations, std::size_t channels)
{
	m_azi = azimuths;
	m_ele = elevations;
	m_numLsChans = channels;

	m_hrirBuffers.resize(m_numLsChans);
}

void BinauralRenderer::setDecodingMatrix(std::vector<float>& decodeMatrix)
{
	m_decodeMatrix = decodeMatrix;
}

void BinauralRenderer::setHeadTrackingData(float roll, float pitch, float yaw)
{
	m_roll = roll;
	m_pitch = pitch;
	m_yaw = yaw;

	m_headTrackRotator.updateEuler(m_roll, m_pitch, m_yaw);

	convertHRIRToSHDHRIR();
}

void BinauralRenderer::setUseSHDConv(bool use)
{
	m_useSHDConv = use;
}

void BinauralRenderer::paint(Graphics& g)
{
}

void BinauralRenderer::resized()
{
	m_ambixFileBrowse.setBounds(10, 10, 150, 30);
	m_useSofa.setBounds(10, 45, 150, 30);

	m_orderSelect.setBounds(170, 10, 150, 30);
	m_sofaFileBrowse.setBounds(170, 45, 150, 30);

	m_enableRotation.setBounds(10, 115, 150, 30);
	m_yAxisVal.setBounds(10, 145, 150, 20);
	m_xAxisVal.setBounds(10, 165, 150, 20);
	m_zAxisVal.setBounds(10, 185, 150, 20);
}

void BinauralRenderer::buttonClicked(Button* buttonClicked)
{
	if (buttonClicked == &m_ambixFileBrowse)
	{
		browseForAmbixConfigFile();
	}
	else if (buttonClicked == &m_useSofa)
	{
		m_sofaFileBrowse.setEnabled(m_useSofa.getToggleState());
		updateHRIRs();
	}
	else if (buttonClicked == &m_sofaFileBrowse)
	{
		browseForSofaFile();
	}
}

void BinauralRenderer::comboBoxChanged(ComboBox* comboBoxChanged)
{
	if (comboBoxChanged == &m_orderSelect)
	{
		sendMsgToLogWindow("Changed to Ambisonic order " + String(m_orderSelect.getSelectedItemIndex() + 1));
		setOrder(m_orderSelect.getSelectedItemIndex() + 1);

		m_decodeMatrix.clear();
		m_azi.clear();
		m_ele.clear();

		m_numLsChans = 0;

		switch (m_orderSelect.getSelectedItemIndex() + 1)
		{
		case 0:
			break;
		case 1:
			for (int i = 0; i < 6; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					m_decodeMatrix.push_back(mtx1order[i][j]);
				}

				m_azi.push_back(az1order[i]);
				m_ele.push_back(el1order[i]);
			}
			break;
		case 2:
			break;
		case 3:
			for (int i = 0; i < 26; ++i)
			{
				for (int j = 0; j < 16; ++j)
				{
					m_decodeMatrix.push_back(mtx3order[i][j]);
				}

				m_azi.push_back(az3order[i]);
				m_ele.push_back(el3order[i]);
			}
			break;
		case 4:
			break;
		case 5:
			for (int i = 0; i < 50; ++i)
			{
				for (int j = 0; j < 36; ++j)
				{
					m_decodeMatrix.push_back(mtx5order[i][j]);
				}

				m_azi.push_back(az5order[i]);
				m_ele.push_back(el5order[i]);
			}
			break;
		case 6:
			break;
		case 7:
			break;
		default:
			break;
		}

		updateMatrices();
		updateHRIRs();
	}
}

void BinauralRenderer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	m_sampleRate = sampleRate;

	if (samplesPerBlockExpected > m_blockSize)
	{
		m_blockSize = samplesPerBlockExpected;

		workingBuffer.setSize(64, m_blockSize);
		convBuffer.setSize(2, m_blockSize);
	}
}

void BinauralRenderer::processBlock(AudioBuffer<float>& buffer)
{
	if (m_isConfigChanging)
		return;

	workingBuffer.clear();

	int numSamps = buffer.getNumSamples();

	if (m_useSHDConv)
	{
		for (int c = 0; c < buffer.getNumChannels(); ++c)
			workingBuffer.copyFrom(c, 0, buffer.getReadPointer(c), buffer.getNumSamples());

		buffer.clear();

		if (m_shdConvEngines.size() != m_numAmbiChans)
		{
			// not enough convolution engines to perform this process
			return;
		}
	}
	else
	{
		if ((m_convEngines.size() != m_numLsChans) || (m_numLsChans == 0))
		{
			// not enough convolution engines to perform this process
			return;
		}

		const float** in = buffer.getArrayOfReadPointers();
		float** out = workingBuffer.getArrayOfWritePointers();

		for (int i = 0; i < m_numLsChans; ++i)
		{
			for (int j = 0; j < m_numAmbiChans; ++j)
			{
				for (int k = 0; k < numSamps; ++k)
				{
					out[i][k] += in[j][k] * m_decodeMatrix[(i * m_numAmbiChans) + j];
				}
			}
		}

		buffer.clear();
	}

	convBuffer.clear();

	if (m_useSHDConv)
	{
		for (int i = 0; i < m_numAmbiChans; ++i)
		{
			for (int j = 0; j < 2; ++j)
				convBuffer.copyFrom(j, 0, workingBuffer.getReadPointer(i), numSamps);

			m_shdConvEngines[i]->Add(convBuffer.getArrayOfWritePointers(), convBuffer.getNumSamples(), 2);
			int availSamples = jmin((int)m_shdConvEngines[i]->Avail(numSamps), numSamps);

			if (availSamples > 0)
			{
				float* convo = nullptr;

				for (int k = 0; k < 2; ++k)
				{
					convo = m_shdConvEngines[i]->Get()[k];
					buffer.addFrom(k, 0, convo, numSamps);
				}

				m_shdConvEngines[i]->Advance(availSamples);
			}
		}
	}
	else
	{
		for (int i = 0; i < m_numLsChans; ++i)
		{
			for (int j = 0; j < 2; ++j)
				convBuffer.copyFrom(j, 0, workingBuffer.getReadPointer(i), numSamps);

			m_convEngines[i]->Add(convBuffer.getArrayOfWritePointers(), convBuffer.getNumSamples(), 2);
			int availSamples = jmin((int)m_convEngines[i]->Avail(numSamps), numSamps);

			if (availSamples > 0)
			{
				float* convo = nullptr;

				for (int k = 0; k < 2; ++k)
				{
					convo = m_convEngines[i]->Get()[k];
					buffer.addFrom(k, 0, convo, numSamps);
				}

				m_convEngines[i]->Advance(availSamples);
			}
		}
	}
}

void BinauralRenderer::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
	processBlock(*bufferToFill.buffer);
}

void BinauralRenderer::releaseResources()
{
}

void BinauralRenderer::sendMsgToLogWindow(const String& message)
{
	m_currentLogMessage += message + "\n";
	sendChangeMessage(); // broadcast change message to inform and update the editor
}

void BinauralRenderer::browseForAmbixConfigFile()
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

void BinauralRenderer::browseForSofaFile()
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

void BinauralRenderer::loadAmbixConfigFile(const File& file)
{
	m_isConfigChanging = true;

	FileInputStream fis(file);

	StringArray lines;

	file.readLines(lines);

	while (!fis.isExhausted())
	{
		String line = fis.readNextLine();

		if (line.contains("#GLOBAL"))
		{
			// this is the globals line
			// the following lines contain global configuration details
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
			m_hrirBuffers.clear();

			m_shdConvEngines.clear();
			m_convEngines.clear();

			m_numLsChans = 0;
			m_numHrirLoaded = 0;

			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
					break;

				String path = file.getParentDirectory().getFullPathName();

				File hrtfFile(path + File::getSeparatorString() + line);

				if (hrtfFile.existsAsFile())
				{
					loadHRIRFileToEngine(hrtfFile);
				}
				else
				{
					Logger::outputDebugString(hrtfFile.getFullPathName() + " does not exist");
				}
			}
		}
		else if (line.contains("#DECODERMATRIX"))
		{
			m_decodeMatrix.clear();
			m_encodeMatrix.clear();

			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
					break;

				juce::String::CharPointerType charptr = line.getCharPointer();

				while (charptr != charptr.findTerminatingNull())
				{
					float nextval = static_cast<float>(CharacterFunctions::readDoubleValue(charptr));
					m_decodeMatrix.push_back(nextval);
				}
			}
		}
	}

	if (m_useSHDConv)
	{
		updateSHDHRIRSizes();
		updateMatrices();
		convertHRIRToSHDHRIR();
		uploadSHDHRIRToEngine();
	}
	else
	{
		uploadHRIRToEngine();
	}

	sendMsgToLogWindow("Ambix Config file " + file.getFileName() + " was loaded");

	m_isConfigChanging = false;
}

void BinauralRenderer::loadSofaFile(const File& file)
{
	m_isConfigChanging = true;
	m_sofaFilePath = file.getFullPathName();
	m_hrirBuffers.clear();
	m_shdConvEngines.clear();

	m_numLsChans = 0;
	m_numHrirLoaded = 0;

	updateHRIRs();

	sendMsgToLogWindow("SOFA file " + file.getFileName() + " was loaded");

	m_isConfigChanging = false;
}

void BinauralRenderer::updateHRIRs()
{
	if (m_useSofa.getToggleState())
	{
		if (m_sofaFilePath.isNotEmpty())
		{
			SOFAReader reader(m_sofaFilePath.toStdString());

			std::vector<float> HRIRData;

			std::size_t channels = reader.getNumImpulseChannels();
			std::size_t samples = reader.getNumImpulseSamples();

			for (int i = 0; i < m_numLsChans; ++i)
			{
				if (reader.getResponseForSpeakerPosition(HRIRData, m_azi[i], m_ele[i]))
				{
					AudioBuffer<float> inputBuffer(static_cast<int>(channels), static_cast<int>(samples));

					for (int c = 0; c < channels; ++c)
						inputBuffer.copyFrom(c, 0, HRIRData.data(), samples);

					loadHRIRToEngine(inputBuffer, reader.getSampleRate());
					sendMsgToLogWindow(String(m_azi[i]) + ", " + String(m_ele[i]) + " loaded to the engine");
				}
				else
				{
					sendMsgToLogWindow(String(m_azi[i]) + ", " + String(m_ele[i]) + " could not be loaded to the engine");
				}
			}
		}
	}
	else
	{
		String filename;
		File sourcePath = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("SALTE").getChildFile("48K_24bit");

		for (int i = 0; i < m_numLsChans; ++i)
		{
			filename = "azi_" + String(m_azi[i], 1).replaceCharacter('.', ',') + "_ele_" + String(m_ele[i], 1).replaceCharacter('.', ',') + ".wav";

			if (loadHRIRFileToEngine(sourcePath.getChildFile(filename)))
			{
				sendMsgToLogWindow(filename + " loaded to the engine");
			}
			else
			{
				sendMsgToLogWindow(filename + " could not be loaded to the engine");
			}
		}
	}

	convertHRIRToSHDHRIR();
}

void BinauralRenderer::updateSHDHRIRSizes()
{
	int hrirSamples = m_hrirBuffers[0].getNumSamples();

	for (auto& hrirShdBuffer : m_hrirShdBuffers)
	{
		hrirShdBuffer.setSize(2, hrirSamples);
		hrirShdBuffer.clear();
	}
}

bool BinauralRenderer::loadHRIRFileToEngine(const File& file)
{
	if (file.exists())
	{
		AudioFormatManager formatManager;
		formatManager.registerBasicFormats();
		std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(file));

		AudioBuffer<float> inputBuffer(reader->numChannels, static_cast<int>(reader->lengthInSamples));

		reader->read(&inputBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

		loadHRIRToEngine(inputBuffer, reader->sampleRate);
		return true;
	}
	else
	{
		return false;
	}
}

void BinauralRenderer::loadHRIRToEngine(const AudioBuffer<float>& buffer, const double sampleRate)
{
	m_hrirBuffers.push_back(buffer);
	m_numLsChans++;
	m_numHrirLoaded++;
}

void transpose(std::vector<float>& outmtx, std::vector<float>& inmtx, int rows, int cols)
{
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			outmtx[(j * rows) + i] = inmtx[(i * cols) + j];
		}
	}
}

void BinauralRenderer::updateMatrices()
{
	m_encodeMatrix.resize(m_numLsChans * m_numAmbiChans);
	transpose(m_encodeMatrix, m_decodeMatrix, m_numLsChans, m_numAmbiChans);
}

void BinauralRenderer::convertHRIRToSHDHRIR()
{
	for (int j = 0; j < m_numLsChans; ++j)
		preprocessTranslation(m_hrirBuffers[j], j);

	for (int i = 0; i < m_numAmbiChans; ++i)
	{
		float** out = m_hrirShdBuffers[i].getArrayOfWritePointers();

		for (int j = 0; j < m_numLsChans; ++j)
		{
			const float** in = m_hrirBuffers[j].getArrayOfReadPointers();
			int idx = (i * m_numLsChans) + j;
			const float weight = m_encodeMatrix[idx];

			for (int k = 0; k < m_hrirShdBuffers[i].getNumChannels(); ++k)
				FloatVectorOperations::addWithMultiply(out[k], in[k], weight, m_hrirShdBuffers[i].getNumSamples());
		}
	}
}

void BinauralRenderer::preprocessTranslation(AudioBuffer<float>& buffer, int speakerIndex)
{
	float weight = 1.0f;

	switch (speakerIndex)
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		weight = 1.5f;
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	default:
		break;
	}

	for (int i = 0; i < buffer.getNumChannels(); ++i)
		buffer.applyGain(weight);
}

void BinauralRenderer::uploadSHDHRIRToEngine()
{
	m_shdConvEngines.clear();

	for (int i = 0; i < m_numAmbiChans; ++i)
	{
		WDL_ImpulseBuffer impulseBuffer;
		impulseBuffer.samplerate = m_sampleRate;
		impulseBuffer.SetNumChannels(2);

		for (int m = 0; m < impulseBuffer.GetNumChannels(); ++m)
			impulseBuffer.impulses[m].Set(m_hrirShdBuffers[i].getReadPointer(m), m_hrirShdBuffers[i].getNumSamples());

		std::unique_ptr<WDL_ConvolutionEngine_Div> convEngine = std::make_unique<WDL_ConvolutionEngine_Div>();

		convEngine->SetImpulse(&impulseBuffer);
		convEngine->Reset();

		m_shdConvEngines.push_back(std::move(convEngine));
	}
}

void BinauralRenderer::uploadHRIRToEngine()
{
	m_convEngines.clear();

	for (int i = 0; i < m_numLsChans; ++i)
	{
		preprocessTranslation(m_hrirBuffers[i], i);

		WDL_ImpulseBuffer impulseBuffer;
		impulseBuffer.samplerate = m_sampleRate;
		impulseBuffer.SetNumChannels(2);

		for (int m = 0; m < impulseBuffer.GetNumChannels(); ++m)
			impulseBuffer.impulses[m].Set(m_hrirBuffers[i].getReadPointer(m), m_hrirBuffers[i].getNumSamples());

		std::unique_ptr<WDL_ConvolutionEngine_Div> convEngine = std::make_unique<WDL_ConvolutionEngine_Div>();

		convEngine->SetImpulse(&impulseBuffer);
		convEngine->Reset();

		m_convEngines.push_back(std::move(convEngine));
	}
}

void BinauralRenderer::timerCallback()
{
	if (m_enableRotation.getToggleState())
	{
		m_yAxisVal.setText("Roll: " + String(m_roll, 2) + " deg", dontSendNotification);
		m_xAxisVal.setText("Pitch: " + String(m_pitch, 2) + " deg", dontSendNotification);
		m_zAxisVal.setText("Yaw: " + String(m_yaw, 2) + " deg", dontSendNotification);
	}
}
