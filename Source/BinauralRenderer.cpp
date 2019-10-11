#include "BinauralRenderer.h"

BinauralRenderer::BinauralRenderer()
	: m_order(0)
	, m_numAmbiChans(1)
	, m_numLsChans(0)
	, m_numHrirAdded(0)
	, m_blockSize(0)
	, m_sampleRate(0.0)
	, m_yaw(0.0f)
	, m_pitch(0.0f)
	, m_roll(0.0f)
	, m_xTrans(0.0f)
	, m_yTrans(0.0f)
	, m_zTrans(0.0f)
	, m_enableDualBand(false)
	, m_enableRotation(true)
	, m_enableTranslation(true)
{
}

void BinauralRenderer::init()
{
}

void BinauralRenderer::deinit()
{
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
	const float pi = MathConstants<float>::pi;
	
	// HEAD TRACKING DATA - QUATERNIONS
	if (message.size() == 4 && message.getAddressPattern() == "/rendering/quaternions")
	{
		// change message index order from 0,1,2,3 to match unity coordinates
		float qW = message[0].getFloat32();
		float qX = message[1].getFloat32();
		float qY = message[3].getFloat32();
		float qZ = message[2].getFloat32();

		float roll, pitch, yaw;

		// roll (x-axis rotation)
		float sinp = 2.0f * (qW * qY - qZ * qX);
		
		if (fabs(sinp) >= 1.0f)
			roll = copysign(pi / 2, sinp) * (180.0f / pi); // use 90 degrees if out of range
		else
			roll = asin(sinp) * (180 / pi);

		// pitch (y-axis rotation)
		float sinr_cosp = 2.0f * (qW * qX + qY * qZ);
		float cosr_cosp = 1.0f - 2.0f * (qX * qX + qY * qY);
		pitch = atan2(sinr_cosp, cosr_cosp) * (180.0f / pi);

		// yaw (z-axis rotation)
		float siny_cosp = 2.0f * (qW * qZ + qX * qY);
		float cosy_cosp = 1.0f - 2.0f * (qY * qY + qZ * qZ);
		yaw = atan2(siny_cosp, cosy_cosp) * (180.0f / pi);

		// sign change
		roll = roll * -1.0f;
		pitch = pitch * -1.0f;

		setHeadTrackingData(roll, pitch, yaw);
	}

	if (message.size() == 1 && message.getAddressPattern() == "/rendering/setorder" && message[0].isInt32())
	{
		int order = message[0].getInt32();
		setOrder(order);
	}

	if (message.size() == 3 && message.getAddressPattern() == "/rendering/htrpy")
	{
		float roll = message[0].getFloat32();
		float pitch = message[1].getFloat32();
		float yaw = message[2].getFloat32();

		setHeadTrackingData(roll, pitch, yaw);
	}

	if (message.size() == 1 && message.getAddressPattern() == "/rendering/loadsofa" && message[0].isString())
	{
		File sourcePath = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("SALTE");

		if (sourcePath.exists())
		{
			String filename = message[0].getString();
			sourcePath = sourcePath.getChildFile(filename);

			if (sourcePath.existsAsFile())
			{
				loadHRIRsFromSofaFile(sourcePath, this);
			}
			else
			{
				sendMsgToLogWindow("SOFA file could not be found");
			}
		}
	}
}

int BinauralRenderer::getOrder()
{
	return m_order;
}

void BinauralRenderer::setOrder(const int order)
{
	m_order = order;
	m_numAmbiChans = (order + 1) * (order + 1);
	updateDualBandFilters();
}

void BinauralRenderer::clearVirtualLoudspeakers()
{
	m_azi.clear();
	m_ele.clear();
	m_numLsChans = 0;
}

void BinauralRenderer::setVirtualLoudspeakers(std::vector<float>& azi, std::vector<float>& ele, int chans)
{
	m_azi = azi;
	m_ele = ele;
	m_numLsChans = chans;
}

void BinauralRenderer::getVirtualLoudspeakers(std::vector<float>& azi, std::vector<float>& ele, int& chans)
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

	for (int i = 0; i < 2; ++i)
	{
		lowPassFilters[i].setCoefficients(IIRCoefficients(bLp0, bLp1, bLp2, a0, a1, a2));
		highPassFilters[i].setCoefficients(IIRCoefficients(bHp0, bHp1, bHp2, a0, a1, a2));
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

float BinauralRenderer::getRoll()
{
	return m_roll;
}

float BinauralRenderer::getPitch()
{
	return m_pitch;
}

float BinauralRenderer::getYaw()
{
	return m_yaw;
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
		uploadHRIRsToEngine();
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

	m_workingBuffer.makeCopyOf(buffer);
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

void BinauralRenderer::clearHRIR()
{
	m_hrirBuffers.clear();
	m_hrirShdBuffers.clear();

	m_numHrirAdded = 0;
}

void BinauralRenderer::addHRIR(const AudioBuffer<float>& buffer)
{
	m_hrirBuffers.push_back(buffer);
	m_numHrirAdded++;
}

bool BinauralRenderer::uploadHRIRsToEngine()
{
	ScopedLock lock(m_procLock);

	// convert the discrete HRIRs into SHD HRIRs for improved computational efficiency
	if (!convertHRIRToSHDHRIR())
	{
		sendMsgToLogWindow("could not upload SHD HRIRs to engine as conversion to SHD HRIRs failed");
		return false;
	}

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

	return true;
}

bool BinauralRenderer::convertHRIRToSHDHRIR()
{
	if (m_hrirBuffers.size() <= 0)
	{
		sendMsgToLogWindow("no HRIRs available to convert to SHD HRIRs");
		return false;
	}

	// clear the current SHD HRIR as this process will create new ones
	m_hrirShdBuffers.clear();

	AudioBuffer<float> hrirShdBuffer(2, m_hrirBuffers[0].getNumSamples());
	AudioBuffer<float> basicShdBuffer(2, m_hrirBuffers[0].getNumSamples());
	AudioBuffer<float> maxreShdBuffer(2, m_hrirBuffers[0].getNumSamples());

	for (int i = 0; i < m_numAmbiChans; ++i)
	{
		hrirShdBuffer.clear();
		basicShdBuffer.clear();
		maxreShdBuffer.clear();

		for (int j = 0; j < 2; ++j)
		{
			for (int k = 0; k < m_numLsChans; ++k)
			{
				const int idx = (i * m_numLsChans) + k;

				// apply the appropriate weights to the buffers before cross over filtering below
				const float basicWeight = m_decodeTransposeMatrix[idx];
				basicShdBuffer.addFrom(j, 0, m_hrirBuffers[k], j, 0, m_hrirBuffers[k].getNumSamples(), basicWeight);
				
				const float maxreWeight = m_decodeTransposeMatrix[idx];
				maxreShdBuffer.addFrom(j, 0, m_hrirBuffers[k], j, 0, m_hrirBuffers[k].getNumSamples(), maxreWeight);
			}

			if (m_enableDualBand)
			{
				lowPassFilters[j].reset();
				lowPassFilters[j].processSamples(basicShdBuffer.getWritePointer(j), basicShdBuffer.getNumSamples());
				hrirShdBuffer.addFrom(j, 0, basicShdBuffer, j, 0, basicShdBuffer.getNumSamples());

				highPassFilters[j].reset();
				highPassFilters[j].processSamples(maxreShdBuffer.getWritePointer(j), maxreShdBuffer.getNumSamples());
				hrirShdBuffer.addFrom(j, 0, maxreShdBuffer, j, 0, maxreShdBuffer.getNumSamples());
			}
			else
			{
				hrirShdBuffer.addFrom(j, 0, basicShdBuffer, j, 0, basicShdBuffer.getNumSamples());
			}
		}

		// add the newly created buffer to the member array
		m_hrirShdBuffers.push_back(hrirShdBuffer);
	}

	return true;
}

bool BinauralRenderer::initialiseFromAmbix(const File& ambixFile, BinauralRenderer* renderer)
{
	if (renderer == nullptr)
		return false;

	if (!ambixFile.existsAsFile())
		return false;

	AmbixLoader loader(ambixFile);

	std::vector<float> decodeMatrix;
	loader.getDecodeMatrix(decodeMatrix);
	renderer->setDecodingMatrix(decodeMatrix);

	renderer->clearVirtualLoudspeakers();

	std::vector<float> azi;
	std::vector<float> ele;
	loader.getSourcePositions(azi, ele);
	renderer->setVirtualLoudspeakers(azi, ele, static_cast<int>(azi.size()));

	renderer->clearHRIR();

	for (int i = 0; i < loader.getNumHrirs(); ++i)
	{
		AudioBuffer<float> hrir;
		loader.getHrir(i, hrir);
		renderer->addHRIR(hrir);
	}

	renderer->updateMatrices();
	
	if (!renderer->uploadHRIRsToEngine())
	{
		return false;
	}

	return true;
}

bool BinauralRenderer::loadHRIRsFromSofaFile(const File& sofaFile, BinauralRenderer* renderer)
{
	if (renderer == nullptr)
		return false;

	String sofaFilePath = sofaFile.getFullPathName();

	SOFAReader reader(sofaFilePath.toStdString());

	std::vector<float> azi;
	std::vector<float> ele;
	int chans = 0;

	renderer->getVirtualLoudspeakers(azi, ele, chans);
	
	// there needs to be some speakers loaded in order to use sofa files
	if (chans <= 0)
		return false;

	renderer->clearHRIR();

	std::vector<float> HRIRData;
	std::size_t channels = reader.getNumImpulseChannels();
	std::size_t samples = reader.getNumImpulseSamples();

	for (int i = 0; i < chans; ++i)
	{
		if (reader.getResponseForSpeakerPosition(HRIRData, azi[i], ele[i]))
		{
			AudioBuffer<float> inputBuffer(static_cast<int>(channels), static_cast<int>(samples));

			for (int c = 0; c < channels; ++c)
				inputBuffer.copyFrom(c, 0, HRIRData.data(), static_cast<int>(samples));

			renderer->addHRIR(inputBuffer);
		}
	}

	if (!renderer->uploadHRIRsToEngine())
	{
		return false;
	}

	return true;
}
