#include "LoudspeakerRenderer.h"

LoudspeakerRenderer::LoudspeakerRenderer()
	: m_order(0)
	, m_numAmbiChans(0)
	, m_numLsChans(0)
	, m_blockSize(0)
	, m_sampleRate(0.0)
	, m_enableRenderer(false)
{
}

void LoudspeakerRenderer::sendMsgToLogWindow(String message)
{
	m_currentLogMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
}

void LoudspeakerRenderer::setDecodingMatrix(std::vector<float>& decodeMatrix)
{
	ScopedLock lock(m_procLock);

	m_basicDecodeMatrix = decodeMatrix;
}

void LoudspeakerRenderer::updateMatrices()
{
	ScopedLock lock(m_procLock);

	//m_basicDecodeTransposeMatrix.resize(m_numAmbiChans * m_numLsChans);
	//mat_trans(m_basicDecodeTransposeMatrix.data(), m_basicDecodeMatrix.data(), m_numLsChans, m_numAmbiChans);
}

bool LoudspeakerRenderer::isRendererEnabled()
{
	return m_enableRenderer;
}

void LoudspeakerRenderer::enableRenderer(bool enable)
{
	m_enableRenderer = enable;
}

void LoudspeakerRenderer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	if (sampleRate != m_sampleRate)
	{
		m_sampleRate = sampleRate;
	}

	if (samplesPerBlockExpected != m_blockSize)
	{
		m_blockSize = samplesPerBlockExpected;
		m_workingBuffer.setSize(64, m_blockSize);
	}
}

void LoudspeakerRenderer::processBlock(AudioBuffer<float>& buffer)
{
	ScopedLock lock(m_procLock);

	if (!m_enableRenderer || m_numAmbiChans == 0 || m_numLsChans == 0)
		return;

	m_workingBuffer.clear();

	for (int i = 0; i < m_numLsChans; ++i)
	{
		for (int j = 0; j < m_numAmbiChans; ++j)
		{
			const int idx = (i * m_numAmbiChans) + j;

			const float basicMatrixGain = m_basicDecodeMatrix[idx];
			m_workingBuffer.addFrom(i, 0, buffer.getWritePointer(j), buffer.getNumSamples(), basicMatrixGain);
		}
	}

	buffer.makeCopyOf(m_workingBuffer);
}

void LoudspeakerRenderer::releaseResources()
{
}

void LoudspeakerRenderer::loadAmbixFile(const File& ambixFile)
{
	if (!ambixFile.existsAsFile())
	{
		sendMsgToLogWindow("failed to load: " + ambixFile.getFileName());
		return;
	}

	m_currentAmbixFile = ambixFile;

	AmbixLoader loader(ambixFile);

	//std::vector<float> azi;
	//std::vector<float> ele;
	//loader.getSourcePositions(azi, ele);

	std::vector<float> decodeMatrix;
	loader.getDecodeMatrix(decodeMatrix);
	setDecodingMatrix(decodeMatrix);
	updateMatrices();

	m_numAmbiChans = loader.getNumAmbiChans();
	m_numLsChans = loader.getNumLsChans();

	sendMsgToLogWindow("successfully loaded: " + ambixFile.getFileName() + " / In: " + String(m_numAmbiChans) + " / Out: " + String(m_numLsChans));
}

String LoudspeakerRenderer::getCurrentAmbixFileName()
{
	return m_currentAmbixFile.getFileName();
}