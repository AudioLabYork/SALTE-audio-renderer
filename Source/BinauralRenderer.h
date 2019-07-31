#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <fftw3.h>
#include <convoengine.h>

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

	void browseForSofaFile();
	void loadSofaFile(File file);

private:
	void doDebugStuff();

	virtual void timerCallback() override;

	TextButton sofaFileBrowse;
	TextButton triggerDebug;

	std::size_t m_order;
	std::size_t m_numAmbiChans;
	std::size_t m_numLsChans;

	std::size_t m_blockSize;
	double m_sampleRate;

	std::vector<float> m_decodeMatrix;
	std::vector<float> m_azimuths;
	std::vector<float> m_elevations;

	AmbisonicRotation m_headTrackRotator;
	
	std::vector<std::unique_ptr<ConvolutionEngine>> m_engines;
	std::vector<std::unique_ptr<WDL_ConvolutionEngine_Div>> m_convEngines;

	float m_yaw;
	float m_pitch;
	float m_roll;

	ToggleButton m_enableRotation;
	Label m_xAxisVal;
	Label m_yAxisVal;
	Label m_zAxisVal;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BinauralRenderer)
};
