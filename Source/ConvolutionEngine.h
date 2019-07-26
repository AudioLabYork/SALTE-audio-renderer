#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include <fftw3.h>

class ConvolutionEngine
{
public:
	ConvolutionEngine();
	~ConvolutionEngine();

	void prepare(std::size_t blockSize);
	void process(float* output, float* input);

	void addResponse(const float* response, const std::size_t length);
	void cleanup();

private:
	void updateSizes();
	void configure();

	std::size_t m_responseSize;
	std::size_t m_blockSize;
	std::size_t m_convSize;

	std::size_t m_numPartitions;
	std::size_t m_partitionSize;

	AudioBuffer<float> m_HRIRdata;

	std::vector<AudioBuffer<float>> signalPartitions;
	std::vector<AudioBuffer<float>> impulsePartitions;

	fftwf_complex* m_impulseFreqBuffer;
	fftwf_complex* m_inputFreqBuffer;
	fftwf_complex* m_sumFreqBuffer;

	fftwf_plan m_r2cPlan; // forward plan
	fftwf_plan m_c2rPlan; // inverse plan

	bool m_isConfigured;
};
