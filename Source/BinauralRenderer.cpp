#include "BinauralRenderer.h"

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
	, m_xTrans(0.0f)
	, m_yTrans(0.0f)
	, m_zTrans(0.0f)
	, m_oscTxRx(nullptr)
	, m_useSHDConv(false)
	, m_enableRotation(true)
	, m_enableTranslation(true)
{
}

void BinauralRenderer::init(OscTransceiver* oscTxRx)
{
	m_oscTxRx = oscTxRx;
	m_oscTxRx->addListener(this);
}

void BinauralRenderer::reset()
{
	m_order = 0;
	m_numAmbiChans = 1;
	m_numLsChans = 0;
	m_numHrirLoaded = 0;
	m_blockSize = 0;
	m_sampleRate = 0.0;

	m_azi.clear();
	m_ele.clear();
}

void BinauralRenderer::deinit()
{
}

void BinauralRenderer::loadFromAmbixConfigFile(const File& file)
{
	FileInputStream fis(file);

	if (!fis.openedOk())
		return;

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
			clearLoudspeakerChannels();
			clearHRIR();

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

					addHRIR(inputBuffer);
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
					setDecodingMatrix(decodeMatrix);
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

	updateMatrices();
	preprocessHRIRs();
	uploadHRIRsToEngine();
}

void BinauralRenderer::oscMessageReceived(const OSCMessage& message)
{
	processOscMessage(message);
}

void BinauralRenderer::oscBundleReceived(const OSCBundle& bundle)
{
	OSCBundle::Element elem = bundle[0];
	processOscMessage(elem.getMessage());
}

void BinauralRenderer::processOscMessage(const OSCMessage& message)
{
	// HEAD TRACKING DATA - QUATERNIONS
	if (message.size() == 4 && message.getAddressPattern() == "/rendering/quaternions")
	{
		// change message index order from 0,1,2,3 to match unity coordinates
		float qW = message[0].getFloat32();
		float qX = message[1].getFloat32();
		float qY = message[3].getFloat32();
		float qZ = message[2].getFloat32();

		float Roll, Pitch, Yaw;

		// roll (x-axis rotation)
		double sinp = +2.0 * (qW * qY - qZ * qX);
		if (fabs(sinp) >= 1)
			Roll = copysign(double_Pi / 2, sinp) * (180 / double_Pi); // use 90 degrees if out of range
		else
			Roll = asin(sinp) * (180 / double_Pi);

		// pitch (y-axis rotation)
		double sinr_cosp = +2.0 * (qW * qX + qY * qZ);
		double cosr_cosp = +1.0 - 2.0 * (qX * qX + qY * qY);
		Pitch = atan2(sinr_cosp, cosr_cosp) * (180 / double_Pi);

		// yaw (z-axis rotation)
		double siny_cosp = +2.0 * (qW * qZ + qX * qY);
		double cosy_cosp = +1.0 - 2.0 * (qY * qY + qZ * qZ);
		Yaw = atan2(siny_cosp, cosy_cosp) * (180 / double_Pi);

		// Sign change
		Roll = Roll * -1;
		Pitch = Pitch * -1;

		setHeadTrackingData(Roll, Pitch, Yaw);
	}

	if (message.size() == 1 && message.getAddressPattern() == "/rendering/setorder" && message[0].isInt32())
	{
		int order = message[0].getInt32();
		setOrder(order);
	}

	// HEAD TRACKING DATA - ROLL PITCH YAW
	if (message.size() == 3 && message.getAddressPattern() == "/rendering/htrpy")
	{
		float Roll = message[0].getFloat32();
		float Pitch = message[1].getFloat32();
		float Yaw = message[2].getFloat32();

		setHeadTrackingData(Roll, Pitch, Yaw);
	}

	//if (message.size() == 1 && message.getAddressPattern() == "/rendering/loadsofa" && message[0].isString())
	//{
	//	File sourcePath = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("SALTE");

	//	if (sourcePath.exists())
	//	{
	//		String filename = message[0].getString();
	//		sourcePath = sourcePath.getChildFile(filename);

	//		if (sourcePath.existsAsFile())
	//		{
	//			brv.loadSofaFile(sourcePath);
	//		}
	//		else
	//		{
	//			Logger::outputDebugString("SOFA file could not be located, please check that it exists");
	//		}
	//	}
	//}
}

void BinauralRenderer::setOrder(const int order)
{
	m_order = order;
	m_numAmbiChans = (order + 1) * (order + 1);

	m_hrirShdBuffers.resize(m_numAmbiChans);
}

void BinauralRenderer::clearLoudspeakerChannels()
{
	m_azi.clear();
	m_ele.clear();
	m_numLsChans = 0;
}

void BinauralRenderer::setLoudspeakerChannels(std::vector<float>& azi, std::vector<float>& ele, int chans)
{
	m_azi = azi;
	m_ele = ele;
	m_numLsChans = chans;

	m_hrirBuffers.resize(m_numLsChans);
}

void BinauralRenderer::getLoudspeakerChannels(std::vector<float>& azi, std::vector<float>& ele, int& chans)
{
	azi = m_azi;
	ele = m_ele;
	chans = m_numLsChans;
}

void BinauralRenderer::setDecodingMatrix(std::vector<float>& decodeMatrix)
{
	ScopedLock lock(m_procLock);
	m_decodeMatrix = decodeMatrix;
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
	ScopedLock lock(m_procLock);
	m_encodeMatrix.resize(m_numLsChans * m_numAmbiChans);
	transpose(m_encodeMatrix, m_decodeMatrix, m_numLsChans, m_numAmbiChans);
}

void BinauralRenderer::setHeadTrackingData(float roll, float pitch, float yaw)
{
	// swap the rotation direction
	m_roll = -roll;
	m_pitch = -pitch;
	m_yaw = -yaw;

	m_headTrackRotator.updateEulerRPY(m_roll, m_pitch, m_yaw);
}

void BinauralRenderer::setUseSHDConv(bool use)
{
	m_useSHDConv = use;
}

void BinauralRenderer::enableRotation(bool enable)
{
	m_enableRotation = enable;
}

void BinauralRenderer::enableTranslation(bool enable)
{
	m_enableTranslation = enable;
}

void BinauralRenderer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	m_sampleRate = sampleRate;

	if (samplesPerBlockExpected > m_blockSize)
	{
		m_blockSize = samplesPerBlockExpected;

		m_workingBuffer.setSize(64, m_blockSize);
		m_convBuffer.setSize(2, m_blockSize);
	}
}

void BinauralRenderer::processBlock(AudioBuffer<float>& buffer)
{
	ScopedLock lock(m_procLock);

	if (m_enableRotation)
		m_headTrackRotator.process(buffer);

	m_workingBuffer.clear();

	for (int c = 0; c < buffer.getNumChannels(); ++c)
		m_workingBuffer.copyFrom(c, 0, buffer.getReadPointer(c), buffer.getNumSamples());

	m_convBuffer.clear();

	if (m_useSHDConv)
	{
		buffer.clear();

		if ((m_shdConvEngines.size() != m_numAmbiChans) || (m_shdConvEngines.size() == 0))
		{
			// not enough convolution engines to perform this process
			return;
		}

		for (int i = 0; i < m_numAmbiChans; ++i)
		{
			for (int j = 0; j < 2; ++j)
				m_convBuffer.copyFrom(j, 0, m_workingBuffer.getReadPointer(i), buffer.getNumSamples());

			m_shdConvEngines[i]->Add(m_convBuffer.getArrayOfWritePointers(), m_convBuffer.getNumSamples(), 2);
			
			int availSamples = jmin((int)m_shdConvEngines[i]->Avail(buffer.getNumSamples()), buffer.getNumSamples());

			if (availSamples > 0)
			{
				float* convo = nullptr;

				for (int k = 0; k < 2; ++k)
				{
					convo = m_shdConvEngines[i]->Get()[k];
					buffer.addFrom(k, 0, convo, availSamples);
				}

				m_shdConvEngines[i]->Advance(availSamples);
			}
		}
	}
	else
	{
		if ((m_convEngines.size() != m_numLsChans) || (m_convEngines.size() == 0))
		{
			// not enough convolution engines to perform this process
			buffer.clear();
			return;
		}

		const float** in = buffer.getArrayOfReadPointers();
		float** out = m_workingBuffer.getArrayOfWritePointers();

		for (int i = 0; i < m_numLsChans; ++i)
		{
			for (int j = 0; j < m_numAmbiChans; ++j)
			{
				m_workingBuffer.addFrom(i, 0, in[j], buffer.getNumSamples(), m_decodeMatrix[(i * m_numAmbiChans) + j]);
			}
		}

		buffer.clear();

		for (int i = 0; i < m_numLsChans; ++i)
		{
			for (int j = 0; j < 2; ++j)
				m_convBuffer.copyFrom(j, 0, m_workingBuffer.getReadPointer(i), buffer.getNumSamples());

			m_convEngines[i]->Add(m_convBuffer.getArrayOfWritePointers(), m_convBuffer.getNumSamples(), 2);
			
			int availSamples = jmin((int)m_convEngines[i]->Avail(buffer.getNumSamples()), buffer.getNumSamples());

			if (availSamples > 0)
			{
				float* convo = nullptr;

				for (int k = 0; k < 2; ++k)
				{
					convo = m_convEngines[i]->Get()[k];
					buffer.addFrom(k, 0, convo, buffer.getNumSamples());
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

void BinauralRenderer::loadFromSofaFile(const File& file)
{
	String sofaFilePath = file.getFullPathName();

	SOFAReader reader(sofaFilePath.toStdString());

	std::vector<float> HRIRData;
	std::size_t channels = reader.getNumImpulseChannels();
	std::size_t samples = reader.getNumImpulseSamples();

	std::vector<float> azi;
	std::vector<float> ele;
	int chans = 0;

	getLoudspeakerChannels(azi, ele, chans);

	clearHRIR();

	for (int i = 0; i < chans; ++i)
	{
		if (reader.getResponseForSpeakerPosition(HRIRData, azi[i], ele[i]))
		{
			AudioBuffer<float> inputBuffer(static_cast<int>(channels), static_cast<int>(samples));

			for (int c = 0; c < channels; ++c)
				inputBuffer.copyFrom(c, 0, HRIRData.data(), static_cast<int>(samples));

			addHRIR(inputBuffer);
		}
	}

	preprocessHRIRs();
	uploadHRIRsToEngine();
}

void BinauralRenderer::clearHRIR()
{
	m_hrirBuffers.clear();

	for (int i = 0; i < m_hrirShdBuffers.size(); ++i)
		m_hrirShdBuffers[i].clear();

	m_numLsChans = 0;
	m_numHrirLoaded = 0;
}

void BinauralRenderer::addHRIR(const AudioBuffer<float>& buffer)
{
	m_hrirBuffers.push_back(buffer);
	m_numLsChans++;
	m_numHrirLoaded++;
}

void BinauralRenderer::preprocessHRIRs()
{
	for (int i = 0; i < m_hrirBuffers.size(); ++i)
	{
		float weight = 1.0f;

		switch (i)
		{
		case 0:
			break;
		case 1:
			break;
		case 2:
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

		for (int j = 0; j < m_hrirBuffers[i].getNumChannels(); ++j)
			m_hrirBuffers[i].applyGain(weight);
	}
}

void BinauralRenderer::uploadHRIRsToEngine()
{
	ScopedLock lock(m_procLock);

	if (m_useSHDConv)
	{
		convertHRIRToSHDHRIR();

		m_shdConvEngines.clear();

		for (int i = 0; i < m_numAmbiChans; ++i)
		{
			WDL_ImpulseBuffer impulseBuffer;
			impulseBuffer.samplerate = m_sampleRate;
			impulseBuffer.SetNumChannels(2);

			for (int m = 0; m < impulseBuffer.GetNumChannels(); ++m)
				impulseBuffer.impulses[m].Set(m_hrirShdBuffers[i].getReadPointer(m), m_hrirShdBuffers[i].getNumSamples());

			std::unique_ptr<WDL_ConvolutionEngine> convEngine = std::make_unique<WDL_ConvolutionEngine>();
			convEngine->SetImpulse(&impulseBuffer, -1, 0, 0, false);
			convEngine->Reset();

			m_shdConvEngines.push_back(std::move(convEngine));
		}
	}
	else
	{
		m_convEngines.clear();

		for (int i = 0; i < m_numLsChans; ++i)
		{
			WDL_ImpulseBuffer impulseBuffer;
			impulseBuffer.samplerate = m_sampleRate;
			impulseBuffer.SetNumChannels(2);

			for (int m = 0; m < impulseBuffer.GetNumChannels(); ++m)
				impulseBuffer.impulses[m].Set(m_hrirBuffers[i].getReadPointer(m), m_hrirBuffers[i].getNumSamples());

			std::unique_ptr<WDL_ConvolutionEngine> convEngine = std::make_unique<WDL_ConvolutionEngine>();
			convEngine->SetImpulse(&impulseBuffer, -1, 0, 0, false);
			convEngine->Reset();

			m_convEngines.push_back(std::move(convEngine));
		}
	}
}

void BinauralRenderer::convertHRIRToSHDHRIR()
{
	if (m_hrirBuffers.size() <= 0)
		return;

	int hrirSamples = m_hrirBuffers[0].getNumSamples();

	for (auto& hrirShdBuffer : m_hrirShdBuffers)
	{
		hrirShdBuffer.setSize(2, hrirSamples);
		hrirShdBuffer.clear();
	}

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
