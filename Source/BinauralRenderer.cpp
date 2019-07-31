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
{
}

void BinauralRenderer::init()
{
	startTimer(100);

	sofaFileBrowse.setButtonText("Select SOFA file...");
	sofaFileBrowse.addListener(this);
	addAndMakeVisible(&sofaFileBrowse);

	triggerDebug.setButtonText("Trigger debug");
	triggerDebug.addListener(this);
	addAndMakeVisible(&triggerDebug);

	m_enableRotation.setButtonText("Enable rotation");
	addAndMakeVisible(&m_enableRotation);

	m_xAxisVal.setText("Yaw: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_xAxisVal);
	m_yAxisVal.setText("Pitch: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_yAxisVal);
	m_zAxisVal.setText("Roll: " + String(0.0f, 2) + " deg", dontSendNotification);
	addAndMakeVisible(&m_zAxisVal);

	// pinv, maxre, energy preservation decode matrix for first order to stereo decoding
	m_decodeMatrix = {	0.255550625999976, 0.632455532033675, 0, 0.156492159287191,
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

void BinauralRenderer::setHeadTrackingData(float yaw, float pitch, float roll)
{
	m_yaw = yaw;
	m_pitch = pitch;
	m_roll = roll;

	m_headTrackRotator.updateEuler(m_yaw, m_pitch, m_roll);
}

void BinauralRenderer::paint(Graphics& g)
{
}

void BinauralRenderer::resized()
{
	sofaFileBrowse.setBounds(10, 10, 150, 30);
	triggerDebug.setBounds(10, 45, 150, 30);
	m_enableRotation.setBounds(10, 80, 150, 30);
	m_xAxisVal.setBounds(10, 110, 150, 20);
	m_yAxisVal.setBounds(10, 130, 150, 20);
	m_zAxisVal.setBounds(10, 150, 150, 20);
}

void BinauralRenderer::buttonClicked(Button* buttonClicked)
{
	if (buttonClicked == &sofaFileBrowse)
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
	AudioBuffer<float>* buffer = bufferToFill.buffer;

	if (buffer->getNumChannels() < jmax(m_numAmbiChans, m_numLsChans))
	{
		// not enough channels to do this process
		jassertfalse;
		return;
	}

	if (m_decodeMatrix.size() < m_numAmbiChans * m_numLsChans)
	{
		// not enough decode coefficients to do this process
		jassertfalse;
		return;
	}

	if(m_enableRotation.getToggleState())
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
				out[i][k] += in[0][k] * m_decodeMatrix[(i * m_numAmbiChans) + 0];
				out[i][k] += in[1][k] * m_decodeMatrix[(i * m_numAmbiChans) + 1];
				out[i][k] += in[2][k] * m_decodeMatrix[(i * m_numAmbiChans) + 2];
				out[i][k] += in[3][k] * m_decodeMatrix[(i * m_numAmbiChans) + 3];
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

	for (auto& file : filestoload)
	{
		File inputFile(inputFileLocation.getChildFile(file));

		if (inputFile.exists())
		{
			AudioFormatManager formatManager;
			formatManager.registerBasicFormats();
			std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(inputFile));

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
}

void BinauralRenderer::timerCallback()
{
	if (m_enableRotation.getToggleState())
	{
		m_xAxisVal.setText("Yaw: " + String(m_yaw, 2) + " deg", dontSendNotification);
		m_yAxisVal.setText("Pitch: " + String(m_pitch, 2) + " deg", dontSendNotification);
		m_zAxisVal.setText("Roll: " + String(m_roll, 2) + " deg", dontSendNotification);
	}
}
