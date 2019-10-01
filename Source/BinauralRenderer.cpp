#include "BinauralRenderer.h"

BinauralRenderer::BinauralRenderer()
	: m_yaw(0.0f)
	, m_pitch(0.0f)
	, m_roll(0.0f)
	, m_xTrans(0.0f)
	, m_yTrans(0.0f)
	, m_zTrans(0.0f)
	, m_oscTxRx(nullptr)
	, m_order(0)
	, m_numAmbiChans(1)
	, m_numLsChans(0)
	, m_numHrirLoaded(0)
	, m_blockSize(0)
	, m_sampleRate(0.0)
	, m_enableDualBand(false)
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
	sendMsgToLogWindow("Loaded: " + file.getFileName());
}

void BinauralRenderer::sendMsgToLogWindow(String message)
{
	m_currentLogMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
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
	//			m_binauralRendererView.loadSofaFile(sourcePath);
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
	updateDualBandFilters();
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
	m_decodeTransposeMatrix.resize(m_numAmbiChans * m_numLsChans);
	transpose(m_decodeTransposeMatrix, m_decodeMatrix, m_numLsChans, m_numAmbiChans);
}

void BinauralRenderer::updateDualBandFilters()
{
	double fc = (343.0 * m_order) / (4 * 0.088 * (m_order + 1) * sin(MathConstants<double>::pi / (2 * m_order + 2.0)));
	
	if (fc == 0)
		return;

	double k = tan((MathConstants<double>::pi * fc) / m_sampleRate);
	double k2 = 2.0 * k;

	double denom = pow(k, 2) + k2 + 1.0;

	double a0 = 1.0;
	double a1 = (2 * (pow(k, 2.0) - 1.0)) / denom;
	double a2 = (pow(k, 2.0) - k2 + 1.0) / denom;

	double bLp0 = pow(k, 2.0) / denom;
	double bLp1 = 2.0 * bLp0;
	double bLp2 = bLp0;

	double bHp0 = 1.0 / denom;
	double bHp1 = -2.0 * bHp0;
	double bHp2 = bHp0;

	for (int i = 0; i < m_numAmbiChans; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			lowPassFilters[i][j].setCoefficients(IIRCoefficients(bLp0, bLp1, bLp2, a0, a1, a2));
			highPassFilters[i][j].setCoefficients(IIRCoefficients(bHp0, bHp1, bHp2, a0, a1, a2));
		}
	}
}

void BinauralRenderer::setHeadTrackingData(float roll, float pitch, float yaw)
{
	// swap the rotation direction
	m_roll = -roll;
	m_pitch = -pitch;
	m_yaw = -yaw;

	m_headTrackRotator.updateEulerRPY(m_roll, m_pitch, m_yaw);
}

void BinauralRenderer::enableDualBand(bool enable)
{
	m_enableDualBand = enable;
	uploadHRIRsToEngine();
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

	if (sampleRate != m_sampleRate)
	{
		m_sampleRate = sampleRate;

		updateDualBandFilters();
	}

	if (samplesPerBlockExpected != m_blockSize)
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

void BinauralRenderer::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
	if (bufferToFill.buffer->getNumChannels() <= 2)
		return;

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

void BinauralRenderer::convertHRIRToSHDHRIR()
{
	if (m_hrirBuffers.size() <= 0)
		return;

	AudioBuffer<float> basicShdBuffer(2, m_hrirBuffers[0].getNumSamples());
	AudioBuffer<float> maxreShdBuffer(2, m_hrirBuffers[0].getNumSamples());

	for (int i = 0; i < m_numAmbiChans; ++i)
	{
		m_hrirShdBuffers[i].setSize(2, m_hrirBuffers[0].getNumSamples());
		m_hrirShdBuffers[i].clear();
		basicShdBuffer.clear();
		maxreShdBuffer.clear();

		for (int j = 0; j < m_numLsChans; ++j)
		{
			const int idx = (i * m_numLsChans) + j;
			const float basicWeight = m_decodeTransposeMatrix[idx];
			const float maxreWeight = m_decodeTransposeMatrix[idx];

			// apply the appropriate weights to the buffers before cross over filtering below
			for (int k = 0; k < m_hrirShdBuffers[i].getNumChannels(); ++k)
			{
				basicShdBuffer.addFrom(k, 0, m_hrirBuffers[j], k, 0, m_hrirBuffers[j].getNumSamples(), basicWeight);
				maxreShdBuffer.addFrom(k, 0, m_hrirBuffers[j], k, 0, m_hrirBuffers[j].getNumSamples(), maxreWeight);
			}
		}

		for (int k = 0; k < m_hrirShdBuffers[i].getNumChannels(); ++k)
		{
			if (m_enableDualBand)
			{
				lowPassFilters[i][k].processSamples(basicShdBuffer.getWritePointer(k), basicShdBuffer.getNumSamples());
				highPassFilters[i][k].processSamples(maxreShdBuffer.getWritePointer(k), maxreShdBuffer.getNumSamples());
				
				m_hrirShdBuffers[i].addFrom(k, 0, basicShdBuffer, k, 0, basicShdBuffer.getNumSamples());
				m_hrirShdBuffers[i].addFrom(k, 0, maxreShdBuffer, k, 0, maxreShdBuffer.getNumSamples());
			}
			else
			{
				m_hrirShdBuffers[i].addFrom(k, 0, basicShdBuffer, k, 0, basicShdBuffer.getNumSamples());
			}
		}
	}
}
