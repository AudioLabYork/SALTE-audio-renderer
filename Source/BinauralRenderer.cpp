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
	, m_useSHDConv(false)
	, m_enableRotation(true)
{
}

void BinauralRenderer::init()
{
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

void BinauralRenderer::setDecodingMatrix(std::vector<float>& decodeMatrix)
{
	ScopedLock lock(procLock);
	m_decodeMatrix = decodeMatrix;
}

void BinauralRenderer::getLoudspeakerChannels(std::vector<float>& azi, std::vector<float>& ele, int& chans)
{
	azi = m_azi;
	ele = m_ele;
	chans = m_numLsChans;
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
	ScopedLock lock(procLock);
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
	ScopedLock lock(procLock);

	workingBuffer.clear();

	int numSamps = buffer.getNumSamples();

	if (m_enableRotation)
		m_headTrackRotator.process(buffer);

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

void BinauralRenderer::clearHRIR()
{
	m_hrirBuffers.clear();
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

			std::unique_ptr<WDL_ConvolutionEngine_Div> convEngine = std::make_unique<WDL_ConvolutionEngine_Div>();

			convEngine->SetImpulse(&impulseBuffer);
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

			std::unique_ptr<WDL_ConvolutionEngine_Div> convEngine = std::make_unique<WDL_ConvolutionEngine_Div>();

			convEngine->SetImpulse(&impulseBuffer);
			convEngine->Reset();

			m_convEngines.push_back(std::move(convEngine));
		}
	}
}

void BinauralRenderer::convertHRIRToSHDHRIR()
{
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
