#include "BinauralRenderer.h"

BinauralRenderer::BinauralRenderer()
	: m_order(1)
	, m_numAmbiChans(4)
	, m_numLsChans(2)
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

	ambixFileBrowse.setButtonText("Select Ambix Config file...");
	ambixFileBrowse.addListener(this);
	addAndMakeVisible(&ambixFileBrowse);

	sofaFileBrowse.setButtonText("Select SOFA file...");
	sofaFileBrowse.addListener(this);
	addAndMakeVisible(&sofaFileBrowse);

	triggerDebug.setButtonText("Trigger debug");
	triggerDebug.addListener(this);
	addAndMakeVisible(&triggerDebug);

	m_enableRotation.setButtonText("Enable rotation");
	addAndMakeVisible(&m_enableRotation);

	m_yAxisVal.setText("Roll: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_xAxisVal);
	m_xAxisVal.setText("Pitch: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_yAxisVal);
	m_zAxisVal.setText("Yaw: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_zAxisVal);

	// pinv, maxre, energy preservation decode matrix for first order to stereo decoding
	m_decodeMatrix = { 0.255550625999976, 0.632455532033675, 0, 0.156492159287191,
						0.255550625999976, -0.632455532033675, 0, 0.156492159287190 };

	// default stereo
	m_azimuths = { 30.0f, 330.0f };
	m_elevations = { 0.0f, 0.0f };
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

void BinauralRenderer::setOrder(std::size_t order)
{
	m_order = order;
	m_numAmbiChans = (order + 1) * (order + 1);
}

void BinauralRenderer::setLoudspeakerChannels(std::vector<float>& azimuths, std::vector<float>& elevations, std::size_t channels)
{
	m_azimuths = azimuths;
	m_elevations = elevations;
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
	ambixFileBrowse.setBounds(10, 10, 150, 30);
	sofaFileBrowse.setBounds(10, 45, 150, 30);
	triggerDebug.setBounds(10, 80, 150, 30);
	m_enableRotation.setBounds(10, 115, 150, 30);
	m_yAxisVal.setBounds(10, 145, 150, 20);
	m_xAxisVal.setBounds(10, 165, 150, 20);
	m_zAxisVal.setBounds(10, 185, 150, 20);
}

void BinauralRenderer::buttonClicked(Button* buttonClicked)
{
	if (buttonClicked == &ambixFileBrowse)
	{
		browseForAmbixConfigFile();
	}
	else if (buttonClicked == &sofaFileBrowse)
	{
		browseForSofaFile();
	}
	else if (buttonClicked == &triggerDebug)
	{
		doDebugStuff();
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
	{
		return;
	}

	AudioBuffer<float>* buffer = bufferToFill.buffer;

	if (m_decodeMatrix.size() < m_numAmbiChans * m_numLsChans)
	{
		// not enough decode coefficients to do this process
		jassertfalse;
		return;
	}

	if (m_enableRotation.getToggleState())
		m_headTrackRotator.process(*buffer);

	AudioBuffer<float> workingBuffer(m_numLsChans, buffer->getNumSamples());
	workingBuffer.clear();

	const float** in = buffer->getArrayOfReadPointers();
	float** out = workingBuffer.getArrayOfWritePointers();

	// decode ambisonics into virtual loudspeakers
	int numSamps = buffer->getNumSamples();

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

	// for each loudspeaker channel, need to perform the convolution
	// the number of engines should equal the number of loudspeaker channels
	if (m_convEngines.size() != m_numLsChans)
	{
		// not enough convolution engines to perform this process
		return;
	}

	// clear the buffer ready for output
	buffer->clear();

	for (int i = 0; i < m_numLsChans; ++i)
	{
		// create a new stereo buffer because each convolution will be using one source speaker,
		// but it will be convolved with a stereo impulse response
		AudioBuffer<float> convBuffer(2, numSamps);
		convBuffer.clear();

		int numConvChans = convBuffer.getNumChannels();

		for (int j = 0; j < numConvChans; ++j)
		{
			// copy the loudspeaker channel into both sides of the convolution buffer
			// 0.5f is maybe correct... or should it be 0.707......
			convBuffer.copyFrom(j, 0, out[i], numSamps, 0.5f);
		}

		// add the convolution buffer to the engine in preperation for convolution process
		m_convEngines[i]->Add(convBuffer.getArrayOfWritePointers(), convBuffer.getNumSamples(), numConvChans);

		int availSamples = jmin((int)m_convEngines[i]->Avail(numSamps), numSamps);

		if (availSamples > 0)
		{
			float* convo = nullptr;

			// this needs to be number of channels of impulse
			for (int k = 0; k < numConvChans; ++k)
			{
				// get the results from convolution
				convo = m_convEngines[i]->Get()[k];
				// need to copy back the results
				convBuffer.copyFrom(k, 0, convo, availSamples);
			}

			m_convEngines[i]->Advance(availSamples);
		}

		// sum to the left and right outputs
		for (int m = 0; m < numConvChans; ++m)
		{
			buffer->addFrom(m, 0, convBuffer.getWritePointer(m), numSamps);
		}
	}
}

void BinauralRenderer::releaseResources()
{
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

void BinauralRenderer::loadAmbixConfigFile(File file)
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
				String line = fis.readNextLine().trim();

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

			// clean up the engines
			m_convEngines.clear();
			m_numLsChans = 0;

			while (!fis.isExhausted())
			{
				String line = fis.readNextLine().trim();

				if (line.contains("#END"))
					break;

				String path = file.getParentDirectory().getFullPathName();

				File hrtfFile(path + File::getSeparatorString() + line);

				if (hrtfFile.existsAsFile())
				{
					loadHRIRFileToEngine(hrtfFile);
					m_numLsChans++;
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

	m_isConfigChanging = false;
}

void BinauralRenderer::loadSofaFile(File file)
{
	// TODO: need to clean up the current configuration to deinit all the current hrir units etc

	SOFAReader reader(file.getFullPathName().toStdString());

	std::vector<float> HRIRData;

	std::size_t channels = reader.getNumImpulseChannels();

	for (int i = 0; i < m_numLsChans; ++i)
	{
		reader.getResponseForSpeakerPosition(HRIRData, m_azimuths[i], m_elevations[i]);

		// WDL buffers and engines
		WDL_ImpulseBuffer impulseBuffer;

		impulseBuffer.SetNumChannels(channels);

		for (int c = 0; c < channels; ++c)
			impulseBuffer.impulses[c].Set(HRIRData.data(), HRIRData.size());

		std::unique_ptr<WDL_ConvolutionEngine_Div> convEngine = std::make_unique<WDL_ConvolutionEngine_Div>();

		convEngine->SetImpulse(&impulseBuffer);
		convEngine->Reset();

		m_convEngines.push_back(std::move(convEngine));
	}
}

void BinauralRenderer::loadHRIRFileToEngine(File file)
{
	if (file.exists())
	{
		AudioFormatManager formatManager;
		formatManager.registerBasicFormats();
		std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(file));

		AudioBuffer<float> inputBuffer(reader->numChannels, reader->lengthInSamples);

		reader->read(&inputBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

		// WDL buffers and engines
		WDL_ImpulseBuffer impulseBuffer;
		impulseBuffer.samplerate = reader->sampleRate;
		impulseBuffer.SetNumChannels(reader->numChannels);

		for (int c = 0; c < inputBuffer.getNumChannels(); ++c)
			impulseBuffer.impulses[c].Set(inputBuffer.getWritePointer(c), inputBuffer.getNumSamples());

		std::unique_ptr<WDL_ConvolutionEngine_Div> convEngine = std::make_unique<WDL_ConvolutionEngine_Div>();

		convEngine->SetImpulse(&impulseBuffer);
		convEngine->Reset();

		m_convEngines.push_back(std::move(convEngine));
	}
}

void BinauralRenderer::doDebugStuff()
{
	StringArray filestoload
	{
		"azi_45,0_ele_0,0.wav",
		"azi_135,0_ele_0,0.wav",
		"azi_225,0_ele_0,0.wav",
		"azi_315,0_ele_0,0.wav"
	};

	File inputFileLocation(File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Libraries/Database-Master/D1/D1_HRIR_WAV/48K_24bit/"));
	// File inputFileLocation("D:/TR_FILES/Libraries/Database-Master/D1/D1_HRIR_WAV/48K_24bit/");


	for (auto& file : filestoload)
	{
		File inputFile(inputFileLocation.getChildFile(file));
		loadHRIRFileToEngine(inputFile);
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
