#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <convoengine.h>
#include "AmbisonicRotation.h"
#include "OscTransceiver.h"
#include "SOFAReader.h"
#include "AmbixLoader.h"
#include "ROM.h"
#include "Maths.h"

class BinauralRenderer
	: public ChangeBroadcaster
	, public OSCReceiver
	, public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
	BinauralRenderer();

	void oscMessageReceived(const OSCMessage& message) override;
	void oscBundleReceived(const OSCBundle& bundle) override;
	void processOscMessage(const OSCMessage& message);

	void setOrder(const int order);
	int getOrder() const;

	void clearVirtualLoudspeakers();
	void setVirtualLoudspeakers(const std::vector<float>& azi, const std::vector<float>& ele, const int chans);
	void getVirtualLoudspeakers(std::vector<float>& azi, std::vector<float>& ele, int& chans);

	void setHeadTrackingData(float yaw, float pitch, float roll);

	float getRoll();
	float getPitch();
	float getYaw();

	bool isRendererEnabled();

	void enableRenderer(bool enable);
	void enableDualBand(bool enable);
	void enableRotation(bool enable);

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void processBlock(AudioBuffer<float>& buffer);
	void releaseResources();

	void sendMsgToLogWindow(String message);
	String m_currentLogMessage;

	void clearHRIR();
	void addHRIR(const AudioBuffer<float>& buffer);
	bool uploadHRIRsToEngine();

	void loadStandardDefault();
	void loadAmbixFile(const File& ambixFile);
	void loadHRIRsFromSofaFile(const File& sofaFile);

	class Listener
	{
	public:
		virtual ~Listener() = default;
		virtual void ambixFileLoaded(const File& file) = 0;
		virtual void sofaFileLoaded(const File& file) = 0;
	};

	void addListener(Listener* newListener);
	void removeListener(Listener* listener);

private:
	void setDecodingMatrix(std::vector<float>& decodeMatrix);
	void updateMatrices();

	bool convertHRIRToSHDHRIR();

	void getMaxReWeights(std::vector<float>& weights);

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

	std::vector<AudioBuffer<float>> m_hrirBuffers;
	std::vector<AudioBuffer<float>> m_hrirShdBuffers;

	std::vector<float> m_basicDecodeMatrix;
	std::vector<float> m_weightedDecodeMatrix;
	std::vector<float> m_basicDecodeTransposeMatrix;
	std::vector<float> m_maxreDecodeTransposeMatrix;

	std::vector<float> m_azi;
	std::vector<float> m_ele;

	AmbisonicRotation m_headTrackRotator;

	std::vector<std::unique_ptr<WDL_ConvolutionEngine>> m_convEngines;

	dsp::ProcessorDuplicator<dsp::FIR::Filter<float>, dsp::FIR::Coefficients<float>> m_lowPass;
	dsp::ProcessorDuplicator<dsp::FIR::Filter<float>, dsp::FIR::Coefficients<float>> m_highPass;

	bool m_enableRenderer;
	bool m_enableDualBand;
	bool m_enableRotation;

	ListenerList<Listener> listeners;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinauralRenderer)
};
