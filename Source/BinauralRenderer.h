#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <convoengine.h>
#include "AmbisonicRotation.h"
#include "OscTransceiver.h"
#include "SOFAReader.h"

class BinauralRenderer :	public ChangeBroadcaster,
							public OSCReceiver,
							public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
	BinauralRenderer();

	void init(OscTransceiver* oscTxRx);
	void reset();
	void deinit();

	void loadFromAmbixConfigFile(const File& file);

	void oscMessageReceived(const OSCMessage& message) override;
	void oscBundleReceived(const OSCBundle& bundle) override;
	void processOscMessage(const OSCMessage& message);

	void setOrder(const int order);
	void clearLoudspeakerChannels();
	void setLoudspeakerChannels(std::vector<float>& azi, std::vector<float>& ele, int chans);
	void getLoudspeakerChannels(std::vector<float>& azi, std::vector<float>& ele, int& chans);

	void setDecodingMatrix(std::vector<float>& decodeMatrix);
	void updateMatrices();

	void updateDualBandFilters();

	void setHeadTrackingData(float yaw, float pitch, float roll);
	
	void enableDualBand(bool enable);
	void enableRotation(bool enable);
	void enableTranslation(bool enable);

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void processBlock(AudioBuffer<float>& buffer);
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill);
	void releaseResources();

	void loadFromSofaFile(const File& file);
	void sendMsgToLogWindow(String message);
	String m_currentLogMessage;

	void clearHRIR();
	void addHRIR(const AudioBuffer<float>& buffer);
	void preprocessHRIRs();
	void uploadHRIRsToEngine();

	float m_yaw;
	float m_pitch;
	float m_roll;
	float m_xTrans;
	float m_yTrans;
	float m_zTrans;

private:
	OscTransceiver* m_oscTxRx;

	void convertHRIRToSHDHRIR();

	CriticalSection m_procLock;

	AudioBuffer<float> m_workingBuffer;
	AudioBuffer<float> m_convBuffer;

	int m_order;
	int m_numAmbiChans;
	int m_numLsChans;
	int m_numHrirLoaded;
	int m_blockSize;
	double m_sampleRate;

	std::vector<AudioBuffer<float>> m_hrirBuffers;
	std::vector<AudioBuffer<float>> m_hrirShdBuffers;

	std::vector<float> m_decodeMatrix;
	std::vector<float> m_decodeTransposeMatrix;

	std::vector<float> m_azi;
	std::vector<float> m_ele;

	AmbisonicRotation m_headTrackRotator;

	std::vector<std::unique_ptr<WDL_ConvolutionEngine>> m_convEngines;
	std::vector<std::unique_ptr<WDL_ConvolutionEngine>> m_shdConvEngines;
	
	IIRFilter lowPassFilters[64][2];
	IIRFilter highPassFilters[64][2];

	bool m_enableDualBand;
	bool m_enableRotation;
	bool m_enableTranslation;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinauralRenderer)
};
