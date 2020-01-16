#include "LoudspeakerRenderer.h"

LoudspeakerRenderer::LoudspeakerRenderer()
	: m_order(0)
	, m_numAmbiChans(1)
	, m_numLsChans(0)
	, m_blockSize(0)
	, m_sampleRate(0.0)
	, m_enableRenderer(true)
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

	if (!m_enableRenderer || buffer.getNumChannels() <= 2)
		return;

	m_workingBuffer.makeCopyOf(buffer);

	buffer.clear();
}

void LoudspeakerRenderer::releaseResources()
{
}

void LoudspeakerRenderer::initialiseFromAmbix(const File& ambixFile)
{
	if (!ambixFile.existsAsFile())
		sendMsgToLogWindow("failed to load: " + ambixFile.getFileName());

	AmbixLoader loader(ambixFile);


	std::vector<float> azi;
	std::vector<float> ele;
	loader.getSourcePositions(azi, ele);

	std::vector<float> decodeMatrix;
	loader.getDecodeMatrix(decodeMatrix);
	setDecodingMatrix(decodeMatrix);


	sendMsgToLogWindow("successfully loaded: " + ambixFile.getFileName());

}
