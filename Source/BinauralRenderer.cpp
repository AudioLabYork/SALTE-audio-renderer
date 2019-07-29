#include "BinauralRenderer.h"

BinauralRenderer::BinauralRenderer()
	: m_order(1)
	, m_ambisonicChannels(4)
	, m_loudspeakerChannels(2)
	, m_blockSize(0)
	, m_sampleRate(0.0)
{
}

void BinauralRenderer::init()
{
	sofaFileBrowse.setButtonText("Select SOFA file...");
	sofaFileBrowse.addListener(this);
	addAndMakeVisible(&sofaFileBrowse);

	triggerDebug.setButtonText("Trigger debug");
	triggerDebug.addListener(this);
	addAndMakeVisible(&triggerDebug);

	// pinv, maxre, energy preservation decode matrix for first order to stereo decoding
	m_decodeMatrix = { 0.255550625999976, 0.632455532033675, 0.0, 0.156492159287191,
		0.255550625999976, -0.632455532033675, 0, 0.156492159287190 };

	// default stereo for the time being
	m_azimuths = { 30.0f, 330.0f };
	m_elevations = { 0.0f, 0.0f };
}

void BinauralRenderer::reset()
{
	m_order = 1;
	m_ambisonicChannels = 4;
	m_loudspeakerChannels = 2;
	m_blockSize = 0;
	m_sampleRate = 0.0;
}

void BinauralRenderer::deinit()
{
}

void BinauralRenderer::setOrder(std::size_t order)
{
	m_order = order;
	m_ambisonicChannels = (order + 1) * (order + 1);
}

void BinauralRenderer::setLoudspeakerChannels(std::vector<float>& azimuths, std::vector<float>& elevations, std::size_t channels)
{
	m_loudspeakerChannels = channels;
	m_azimuths = azimuths;
	m_elevations = elevations;
}

void BinauralRenderer::paint(Graphics& g)
{

}

void BinauralRenderer::resized()
{
	sofaFileBrowse.setBounds(10, 10, 150, 30);
	triggerDebug.setBounds(10, 45, 150, 30);
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

	if (buffer->getNumChannels() < std::max(m_ambisonicChannels, m_loudspeakerChannels))
	{
		// not enough channels to do this process
		//jassertfalse;
		return;
	}

	if (m_decodeMatrix.size() < m_ambisonicChannels * m_loudspeakerChannels)
	{
		// not enough decode coefficients to do this process
		//jassertfalse;
		return;
	}

	// TEMP - Return as this processing is not complete yet
	//return;

	AudioBuffer<float> workingBuffer(buffer->getNumChannels(), buffer->getNumSamples());
	workingBuffer.clear();

	const float** in = buffer->getArrayOfReadPointers();
	float** out = workingBuffer.getArrayOfWritePointers();

	// decode ambisonics into virtual loudspeakers
	int numSamples = buffer->getNumSamples();

	for (int i = 0; i < m_loudspeakerChannels; ++i)
	{
		for (int j = 0; j < m_ambisonicChannels; ++j)
		{
			for (int k = 0; k < numSamples; ++k)
			{
				out[i][k] += in[j][k] * m_decodeMatrix[(i * m_loudspeakerChannels) + j];
			}
		}
	}

	for (int c = 0; c < buffer->getNumChannels(); ++c)
	{
		buffer->copyFrom(c, 0, workingBuffer, c, 0, buffer->getNumSamples());
	}

	// process each channel (virtual loudspeaker) with appropriate HRTF
	//for (auto& engine : m_engines)
	//{
	//	engine->process(out[0], out[0]);
	//}
	//
	//for (int i = 0; i < m_loudspeakerChannels; ++i)
	//{
	//	//m_engines[i].process(buffer);
	//}
	//
	//for (int i = 0; i < m_loudspeakerChannels; ++i)
	//{
	//	for (int k = 0; k < numSamples; ++k)
	//	{
	//		// each output
	//		//out[i][k];
	//	}
	//}
}

void BinauralRenderer::releaseResources()
{

}

void BinauralRenderer::ambisonicToBinaural()
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
	// need to clean up the current configuration to deinit all the current hrir units etc

	SOFAReader reader(file.getFullPathName().toStdString());

	std::vector<float> HRIRData;
	AudioBuffer<float> HRIRBuffer;
	
	std::size_t channels = reader.getNumImpulseChannels();
	std::size_t samples = reader.getNumImpulseSamples();

	HRIRBuffer.setSize(channels, samples);

	for (int i = 0; i < m_loudspeakerChannels; ++i)
	{
		std::unique_ptr<ConvolutionEngine> engine = std::make_unique<ConvolutionEngine>();
		engine->prepare(m_blockSize);

		reader.getResponseForSpeakerPosition(HRIRData, m_azimuths[i], m_elevations[i]);

		for (int c = 0; c < channels; ++c)
		{
			float* data = HRIRBuffer.getWritePointer(c);
			FloatVectorOperations::copy(data, HRIRData.data(), samples);
		}

		engine->addResponse(HRIRBuffer.getReadPointer(0), HRIRBuffer.getNumSamples());
		m_engines.push_back(std::move(engine));
	}
}

void BinauralRenderer::doDebugStuff()
{
	StringArray filestoload
	{
		"azi_30,0_ele_0,0.wav",
		"azi_330,0_ele_0,0.wav"
	};

	File inputFileLocation(File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Libraries/Database-Master/D1/D1_HRIR_WAV/48K_24bit/"));

	for (auto& file : filestoload)
	{
		File inputFile(inputFileLocation.getChildFile(file));

		AudioSampleBuffer inputBuffer;
		double inputSampleRate = 0.0;

		if (inputFile.exists())
		{
			AudioFormatManager formatManager;
			formatManager.registerBasicFormats();
			std::unique_ptr<AudioFormatReader> reader(formatManager.createReaderFor(inputFile));

			reader->read(&inputBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
			inputSampleRate = reader->sampleRate;
		}
	}
}



