#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AmbixLoader.h"
#include "Maths.h"

class LoudspeakerRenderer
	: public ChangeBroadcaster
{
public:
	LoudspeakerRenderer();

	void enableRenderer(bool enable);
	bool isRendererEnabled();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void processBlock(AudioBuffer<float>& buffer);
	void releaseResources();

	void sendMsgToLogWindow(String message);
	String m_currentLogMessage;

	void loadAmbixFile(const File& ambixFile);
	String getCurrentAmbixFileName();

private:
	File m_currentAmbixFile;

	void setDecodingMatrix(std::vector<float>& decodeMatrix);
	void updateMatrices();

	std::vector<float> m_basicDecodeMatrix;
	std::vector<float> m_basicDecodeTransposeMatrix;

	CriticalSection m_procLock;

	AudioBuffer<float> m_workingBuffer;

	int m_order;
	int m_numAmbiChans;
	int m_numLsChans;
	int m_blockSize;
	double m_sampleRate;

	bool m_enableRenderer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoudspeakerRenderer)
};
