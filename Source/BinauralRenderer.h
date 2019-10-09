#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <convoengine.h>
#include "AmbisonicRotation.h"
#include "OscTransceiver.h"
#include "SOFAReader.h"
#include "AmbixLoader.h"

class BinauralRenderer :	public ChangeBroadcaster,
							public OSCReceiver,
							public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
	BinauralRenderer();

	void init();
	void deinit();

	void oscMessageReceived(const OSCMessage& message) override;
	void oscBundleReceived(const OSCBundle& bundle) override;
	void processOscMessage(const OSCMessage& message);

	int getOrder();
	void setOrder(const int order);
	void clearVirtualLoudspeakers();
	void setVirtualLoudspeakers(std::vector<float>& azi, std::vector<float>& ele, int chans);
	void getVirtualLoudspeakers(std::vector<float>& azi, std::vector<float>& ele, int& chans);

	void setDecodingMatrix(std::vector<float>& decodeMatrix);
	void updateMatrices();

	void updateDualBandFilters();

	void setHeadTrackingData(float yaw, float pitch, float roll);
	
	float getRoll();
	float getPitch();
	float getYaw();

	void enableDualBand(bool enable);
	void enableRotation(bool enable);
	void enableTranslation(bool enable);

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void processBlock(AudioBuffer<float>& buffer);
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill);
	void releaseResources();

	void sendMsgToLogWindow(String message);
	String m_currentLogMessage;

	void clearHRIR();
	void addHRIR(const AudioBuffer<float>& buffer);
	bool uploadHRIRsToEngine();

	static bool initialiseFromAmbix(const File& ambixFile, BinauralRenderer* renderer);
	static bool loadHRIRsFromSofaFile(const File& sofaFile, BinauralRenderer* renderer);

private:
	bool convertHRIRToSHDHRIR();

	CriticalSection m_procLock;

	AudioBuffer<float> m_workingBuffer;
	AudioBuffer<float> m_convBuffer;

	int m_order;
	int m_numAmbiChans;
	int m_numLsChans;
	int m_numHrirAdded;
	int m_blockSize;
	double m_sampleRate;

	float m_yaw;
	float m_pitch;
	float m_roll;
	float m_xTrans;
	float m_yTrans;
	float m_zTrans;

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
