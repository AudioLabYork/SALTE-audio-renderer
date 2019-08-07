#include "BinauralRenderer.h"
#include "ROM.h"

BinauralRenderer::BinauralRenderer()
	: m_order(1)
	, m_numAmbiChans(4)
	, m_numLsChans(2)
	, m_numHrirLoaded(0)
	, m_blockSize(0)
	, m_sampleRate(0.0)
	, m_yaw(0.0f)
	, m_pitch(0.0f)
	, m_roll(0.0f)
	, m_isConfigChanging(false)
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
	m_order = 1;
	m_numAmbiChans = 4;
	m_numLsChans = 2;
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
}

void BinauralRenderer::setLoudspeakerChannels(std::vector<float>& azimuths, std::vector<float>& elevations, std::size_t channels)
{
	m_azi = azimuths;
	m_ele = elevations;
	m_numLsChans = channels;
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
		resetConvolution();

		setOrder(m_orderSelect.getSelectedItemIndex() + 1);

		m_decodeMatrix.clear();
		m_azi.clear();
		m_ele.clear();

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

			m_numLsChans = 6;
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

			m_numLsChans = 26;
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

			m_numLsChans = 50;
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
		convertResponsesToSHD();
	}
}

void BinauralRenderer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	m_blockSize = samplesPerBlockExpected;
	m_sampleRate = sampleRate;
}

void BinauralRenderer::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
	// maybe need to lock this out if we are changing the config...
	// a flag is used for now
	if (m_isConfigChanging)
		return;

	AudioBuffer<float>* buffer = bufferToFill.buffer;

	// rotating the ambisonic scene
	if (m_enableRotation.getToggleState())
		m_headTrackRotator.process(*buffer);

	// convolving SHD ambisonic input with SHD HRIRs
	AudioBuffer<float> workingBuffer(buffer->getNumChannels(), buffer->getNumSamples());
	workingBuffer.clear();

	for (int c = 0; c < buffer->getNumChannels(); ++c)
		workingBuffer.copyFrom(c, 0, buffer->getReadPointer(c), buffer->getNumSamples());

	int numSamps = buffer->getNumSamples();
	
	// not enough convolution engines to perform this process
	if (m_shdConvEngines.size() != m_numAmbiChans)
		return;

	buffer->clear();
	
	AudioBuffer<float> convBuffer(2, numSamps);
	int numConvChans = convBuffer.getNumChannels();

	for (int i = 0; i < m_numAmbiChans; ++i)
	{
		convBuffer.clear();

		for (int j = 0; j < numConvChans; ++j)
			convBuffer.copyFrom(j, 0, workingBuffer.getReadPointer(i), numSamps, 0.5f);

		m_shdConvEngines[i]->Add(convBuffer.getArrayOfWritePointers(), convBuffer.getNumSamples(), numConvChans);
		
		int availSamples = jmin((int)m_shdConvEngines[i]->Avail(numSamps), numSamps);
		
		if (availSamples > 0)
		{
			float* convo = nullptr;

			for (int k = 0; k < numConvChans; ++k)
			{
				convo = m_shdConvEngines[i]->Get()[k];
				buffer->addFrom(k, 0, convo, numSamps);
			}

			m_shdConvEngines[i]->Advance(availSamples);
		}
	}
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

	resetConvolution();

	// reset number of loudspeakers in configuration
	m_numLsChans = 0;

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
			// this is the hrtf line
			// the following lines contain references to the HRTF files that should be loaded
			// for this configuration

			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
				{
					break;
				}

				String path = file.getParentDirectory().getFullPathName();

				File hrtfFile(path + File::getSeparatorString() + line);

				if (hrtfFile.existsAsFile())
				{
					loadHRIRFileToEngine(hrtfFile);
					m_numLsChans++;
					m_numHrirLoaded++;
				}
				else
				{
					Logger::outputDebugString(hrtfFile.getFullPathName() + " does not exist");
				}
			}
		}
		else if (line.contains("#DECODERMATRIX"))
		{
			// this is the decode matrix line
			// the following lines contain a decode matrix to be used on the ambisonic input
			m_decodeMatrix.clear();
			m_encodeMatrix.clear();

			while (!fis.isExhausted())
			{
				line = fis.readNextLine().trim();

				if (line.contains("#END"))
				{
					break;
				}

				juce::String::CharPointerType charptr = line.getCharPointer();

				while (charptr != charptr.findTerminatingNull())
				{
					float nextval = static_cast<float>(CharacterFunctions::readDoubleValue(charptr));
					m_decodeMatrix.push_back(nextval);
				}
			}
		}
	}

	updateMatrices();
	convertResponsesToSHD();

	sendMsgToLogWindow("Ambix Config file " + file.getFileName() + " was loaded");

	m_isConfigChanging = false;
}

void BinauralRenderer::loadSofaFile(const File& file)
{
	m_isConfigChanging = true;

	resetConvolution();

	SOFAReader reader(file.getFullPathName().toStdString());

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
		}
		else
		{
			Logger::outputDebugString("this HRIR could not be found in the SOFA file");
		}
	}

	convertResponsesToSHD();

	sendMsgToLogWindow("SOFA file " + file.getFileName() + " was loaded");

	m_isConfigChanging = false;
}

void BinauralRenderer::updateHRIRs()
{
	if (m_useSofa.getToggleState())
	{

	}
	else
	{
		String filename;
		File sourcePath = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("SALTE").getChildFile("48K_24bit");

		for (int i = 0; i < m_numLsChans; ++i)
		{
			filename = "azi_" + String(m_azi[i], 1).replaceCharacter('.', ',') + "_ele_" + String(m_ele[i], 1).replaceCharacter('.', ',') + ".wav";
			loadHRIRFileToEngine(sourcePath.getChildFile(filename));
		}
	}
}

void BinauralRenderer::loadHRIRFileToEngine(const File& file)
{
	if (file.exists())
	{
		AudioFormatManager formatManager;
		formatManager.registerBasicFormats();
		std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(file));

		AudioBuffer<float> inputBuffer(reader->numChannels, static_cast<int>(reader->lengthInSamples));

		reader->read(&inputBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

		loadHRIRToEngine(inputBuffer, reader->sampleRate);
	}
}

void BinauralRenderer::loadHRIRToEngine(const AudioBuffer<float>& buffer, const double sampleRate)
{
	m_hrirBuffers.push_back(buffer);

	WDL_ImpulseBuffer impulseBuffer;
	impulseBuffer.samplerate = sampleRate;
	impulseBuffer.SetNumChannels(buffer.getNumChannels());

	for (int c = 0; c < jmin(buffer.getNumChannels(), 2); ++c)
	{
		const float* data = buffer.getReadPointer(c);
		impulseBuffer.impulses[c].Set(data, buffer.getNumSamples());
	}

	std::unique_ptr<WDL_ConvolutionEngine_Div> convEngine = std::make_unique<WDL_ConvolutionEngine_Div>();

	convEngine->SetImpulse(&impulseBuffer);
	convEngine->Reset();

	m_convEngines.push_back(std::move(convEngine));
}


void BinauralRenderer::updateMatrices()
{
	Eigen::MatrixXf decode(m_numLsChans, m_numAmbiChans);

	for (int i = 0; i < m_numLsChans; ++i)
	{
		for (int j = 0; j < m_numAmbiChans; ++j)
		{
			decode(i, j) = m_decodeMatrix[(i * m_numAmbiChans) + j];
		}
	}
	
	Eigen::MatrixXf encode(m_numAmbiChans, m_numLsChans);
	
	encode = decode.transpose();

	for (int i = 0; i < m_numAmbiChans; ++i)
	{
		for (int j = 0; j < m_numLsChans; ++j)
		{
			m_encodeMatrix.push_back(encode(i, j));
		}
	}
}

void BinauralRenderer::convertResponsesToSHD()
{
	if (m_hrirBuffers.size() <= 0)
		return;

	int numSamps = m_hrirBuffers[0].getNumSamples();

	m_hrirShdBuffers.resize(m_numAmbiChans);

	for (auto& hrirShdBuffer : m_hrirShdBuffers)
	{
		hrirShdBuffer.setSize(2, numSamps);
		hrirShdBuffer.clear();
	}

	for (int i = 0; i < m_numAmbiChans; ++i)
	{
		float** out = m_hrirShdBuffers[i].getArrayOfWritePointers();

		for (int j = 0; j < m_numLsChans; ++j)
		{
			const float** in = m_hrirBuffers[j].getArrayOfReadPointers();
			float weight = m_encodeMatrix[(i * m_numLsChans) + j];

			for (int k = 0; k < 2; ++k)
				FloatVectorOperations::addWithMultiply(out[k], in[k], weight, numSamps);
		}

		WDL_ImpulseBuffer impulseBuffer;
		impulseBuffer.samplerate = m_sampleRate;
		impulseBuffer.SetNumChannels(2);

		for (int m = 0; m < 2; ++m)
			impulseBuffer.impulses[m].Set(m_hrirShdBuffers[i].getReadPointer(m), numSamps);

		std::unique_ptr<WDL_ConvolutionEngine_Div> convEngine = std::make_unique<WDL_ConvolutionEngine_Div>();

		convEngine->SetImpulse(&impulseBuffer);
		convEngine->Reset();

		m_shdConvEngines.push_back(std::move(convEngine));
	}
}

void BinauralRenderer::resetConvolution()
{
	// clean up the HRIR buffers
	m_hrirBuffers.clear();
	m_hrirShdBuffers.clear();
	m_shdConvEngines.clear();

	// clean up the engines
	m_convEngines.clear();
	m_shdConvEngines.clear();

	m_numHrirLoaded = 0;
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
