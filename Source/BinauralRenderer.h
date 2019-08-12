#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <convoengine.h>
#include "AmbisonicRotation.h"

class BinauralRenderer
{
public:
	BinauralRenderer();

	void init();
	void reset();
	void deinit();

	void setOrder(const int order);
	void clearLoudspeakerChannels();
	void setLoudspeakerChannels(std::vector<float>& azi, std::vector<float>& ele, int chans);
	void getLoudspeakerChannels(std::vector<float>& azi, std::vector<float>& ele, int& chans);

	void setDecodingMatrix(std::vector<float>& decodeMatrix);
	void updateMatrices();

	void setHeadTrackingData(float yaw, float pitch, float roll);
	void setUseSHDConv(bool use);

	void enableRotation(bool enable);

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void processBlock(AudioBuffer<float>& buffer);
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill);
	void releaseResources();

	void clearHRIR();
	void addHRIR(const AudioBuffer<float>& buffer);
	void preprocessHRIRs();
	void uploadHRIRsToEngine();

	float m_yaw;
	float m_pitch;
	float m_roll;

private:
	void convertHRIRToSHDHRIR();

	CriticalSection procLock;
	AudioBuffer<float> workingBuffer;
	AudioBuffer<float> convBuffer;

	int m_order;
	int m_numAmbiChans;
	int m_numLsChans;
	int m_numHrirLoaded;
	int m_blockSize;
	double m_sampleRate;

	std::vector<AudioBuffer<float>> m_hrirBuffers;
	std::vector<AudioBuffer<float>> m_hrirShdBuffers;

	std::vector<float> m_decodeMatrix;
	std::vector<float> m_encodeMatrix;

	std::vector<float> m_azi;
	std::vector<float> m_ele;

	AmbisonicRotation m_headTrackRotator;

	//std::vector<std::unique_ptr<WDL_ConvolutionEngine_Div>> m_convEngines;
	//std::vector<std::unique_ptr<WDL_ConvolutionEngine_Div>> m_shdConvEngines;
	std::vector<std::unique_ptr<WDL_ConvolutionEngine>> m_convEngines;
	std::vector<std::unique_ptr<WDL_ConvolutionEngine>> m_shdConvEngines;

	bool m_enableRotation;
	bool m_useSHDConv;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinauralRenderer)
};
