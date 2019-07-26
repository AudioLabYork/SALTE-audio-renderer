#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <fftw3.h>

#include "SOFAReader.h"
#include "ConvolutionEngine.h"

class BinauralRenderer
	: public Component
	, public Button::Listener
{
public:
	BinauralRenderer();

	void init();
	void reset();
	void deinit();

	void setOrder(std::size_t order);
	void setLoudspeakerChannels(std::vector<float>& azimuths, std::vector<float>& elevations, std::size_t channels);

	void paint(Graphics& g) override;
	void resized() override;

	void buttonClicked(Button* buttonClicked) override;

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill);
	void releaseResources();

	void ambisonicToBinaural();

	void browseForSofaFile();
	void loadSofaFile(File file);

private:
	void doDebugStuff();

	TextButton sofaFileBrowse;
	TextButton triggerDebug;

	std::size_t m_order;
	std::size_t m_ambisonicChannels;
	std::size_t m_loudspeakerChannels;

	std::size_t m_blockSize;
	double m_sampleRate;

	std::vector<float> m_decodeMatrix;
	std::vector<float> m_azimuths;
	std::vector<float> m_elevations;
	
	std::vector<std::unique_ptr<ConvolutionEngine>> m_engines;
};
