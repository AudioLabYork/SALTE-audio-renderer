#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <fftw3.h>
#include <convoengine.h>
#include <Eigen/Dense>

#include "SOFAReader.h"
#include "ConvolutionEngine.h"
#include "AmbisonicRotation.h"

enum eOrders
{
	k1stOrder,
	k2ndOrder,
	k3rdOrder,
	k4thOrder,
	k5thOrder,
	k6thOrder,
	k7thOrder
};

static StringArray orderChoices = { "1st Order",
									"2nd Order",
									"3rd Order",
									"4th Order",
									"5th Order",
									"6th Order",
									"7th Order" };

class BinauralRenderer
	: public Component
	, public Button::Listener
	, public ComboBox::Listener
	, public Timer
	, public ChangeBroadcaster
{
public:
	BinauralRenderer();

	void init();
	void reset();
	void deinit();

	void setOrder(const int order);
	void setLoudspeakerChannels(std::vector<float>& azimuths, std::vector<float>& elevations, std::size_t channels);
	void setDecodingMatrix(std::vector<float>& decodeMatrix);
	void setHeadTrackingData(float yaw, float pitch, float roll);
	void setUseSHDConv(bool use);

	void paint(Graphics& g) override;
	void resized() override;

	void buttonClicked(Button* buttonClicked) override;
	void comboBoxChanged(ComboBox* comboBoxChanged);

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void processBlock(AudioBuffer<float>& buffer);
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill);
	void releaseResources();
	
	void sendMsgToLogWindow(const String& message);

	void browseForAmbixConfigFile();
	void browseForSofaFile();
	
	void loadAmbixConfigFile(const File& file);
	void loadSofaFile(const File& file);

	String m_currentLogMessage;

private:
	void updateHRIRs();
	void updateSHDHRIRSizes();
	bool loadHRIRFileToEngine(const File& file);
	void loadHRIRToEngine(const AudioBuffer<float>& buffer, const double sampleRate);
	void updateMatrices();
	void convertHRIRToSHDHRIR();
	void preprocessTranslation(AudioBuffer<float>& buffer, int speakerIndex);
	void uploadSHDHRIRToEngine();
	void uploadHRIRToEngine();

	virtual void timerCallback() override;
	
	AudioBuffer<float> workingBuffer;
	AudioBuffer<float> convBuffer;
	
	TextButton m_ambixFileBrowse;
	ToggleButton m_useSofa;

	ComboBox m_orderSelect;
	TextButton m_sofaFileBrowse;

	String m_sofaFilePath;

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

	std::vector<std::unique_ptr<WDL_ConvolutionEngine_Div>> m_convEngines;
	std::vector<std::unique_ptr<WDL_ConvolutionEngine_Div>> m_shdConvEngines;

	float m_yaw;
	float m_pitch;
	float m_roll;

	ToggleButton m_enableRotation;
	Label m_yAxisVal;
	Label m_xAxisVal;
	Label m_zAxisVal;

	bool m_isConfigChanging;
	bool m_useSHDConv;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinauralRenderer)
};
