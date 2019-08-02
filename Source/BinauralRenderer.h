#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <fftw3.h>
#include <convoengine.h>
#include <Eigen/Dense>

#include "SOFAReader.h"
#include "ConvolutionEngine.h"
#include "AmbisonicRotation.h"

class BinauralRenderer
	: public Component
	, public Button::Listener
	, public Timer
{
public:
	BinauralRenderer();

	void init();
	void reset();
	void deinit();

	void setOrder(std::size_t order);
	void setLoudspeakerChannels(std::vector<float>& azimuths, std::vector<float>& elevations, std::size_t channels);
	void setDecodingMatrix(std::vector<float>& decodeMatrix);
	void setHeadTrackingData(float yaw, float pitch, float roll);

	void paint(Graphics& g) override;
	void resized() override;

	void buttonClicked(Button* buttonClicked) override;

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill);
	void releaseResources();
	
	void browseForAmbixConfigFile();
	void browseForSofaFile();

private:
	void loadAmbixConfigFile(File file);
	void loadSofaFile(File file);
	void loadHRIRFileToEngine(File file);
	void updateMatrices();
	void convertResponsesToSHD();
	void doDebugStuff();

	virtual void timerCallback() override;

	TextButton ambixFileBrowse;
	TextButton sofaFileBrowse;
	TextButton triggerDebug;

	std::size_t m_order;
	std::size_t m_numAmbiChans;
	std::size_t m_numLsChans;
	std::size_t m_numHrirLoaded;

	std::size_t m_blockSize;
	double m_sampleRate;

	std::vector<AudioBuffer<float>> m_hrirBuffers;
	std::vector<AudioBuffer<float>> m_hrirShdBuffers;

	std::vector<float> m_decodeMatrix;
	std::vector<float> m_encodeMatrix;

	std::vector<float> m_azimuths;
	std::vector<float> m_elevations;

	AmbisonicRotation m_headTrackRotator;

	std::vector<std::unique_ptr<ConvolutionEngine>> m_engines;
	std::vector<std::unique_ptr<WDL_ConvolutionEngine_Div>> m_convEngines;
	std::vector<std::unique_ptr<WDL_ConvolutionEngine_Div>> m_shdConvEngines;

	float m_yaw;
	float m_pitch;
	float m_roll;

	ToggleButton m_enableRotation;
	Label m_xAxisVal;
	Label m_yAxisVal;
	Label m_zAxisVal;

	bool m_isConfigChanging;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinauralRenderer)
};
