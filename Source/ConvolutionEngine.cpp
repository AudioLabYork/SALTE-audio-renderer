#include "ConvolutionEngine.h"

ConvolutionEngine::ConvolutionEngine()
	: m_responseSize(0)
	, m_blockSize(0)
	, m_convSize(0)
	, m_numPartitions(0)
	, m_partitionSize(64)
	, m_impulseFreqBuffer(nullptr)
	, m_r2cPlan(nullptr)
	, m_c2rPlan(nullptr)
	, m_isConfigured(false)
{

}

ConvolutionEngine::~ConvolutionEngine()
{
	if (m_isConfigured)
		cleanup();
}

void ConvolutionEngine::prepare(std::size_t blockSize)
{
	m_blockSize = blockSize;
	updateSizes();
}

void ConvolutionEngine::process(float* output, float* input)
{
	fftwf_execute_dft_r2c(m_r2cPlan, input, m_inputFreqBuffer);

	for (int s = 0; s < m_convSize; ++s)
	{
		m_sumFreqBuffer[s][0] = (m_inputFreqBuffer[s][0] * m_impulseFreqBuffer[s][0]) - (m_inputFreqBuffer[s][1] * m_impulseFreqBuffer[s][1]);
		m_sumFreqBuffer[s][1] = (m_inputFreqBuffer[s][0] * m_impulseFreqBuffer[s][1]) + (m_inputFreqBuffer[s][1] * m_impulseFreqBuffer[s][0]);
	}

	fftwf_execute_dft_c2r(m_c2rPlan, m_sumFreqBuffer, output);

	FloatVectorOperations::multiply(output, 1.0f / m_convSize, m_convSize); // remember to multiply with weight 1/bins
}

void ConvolutionEngine::addResponse(const float* response, const std::size_t length)
{
	m_responseSize = length;
	updateSizes();

	m_HRIRdata.setSize(1, length);
	float* impulse = m_HRIRdata.getWritePointer(0);
	FloatVectorOperations::clear(impulse, m_convSize);
	FloatVectorOperations::copy(impulse, response, length);

	fftwf_execute_dft_r2c(m_r2cPlan, m_HRIRdata.getWritePointer(0), m_impulseFreqBuffer);



}

void ConvolutionEngine::cleanup()
{
	if (m_r2cPlan)
		fftwf_destroy_plan(m_r2cPlan);

	if (m_c2rPlan)
		fftwf_destroy_plan(m_c2rPlan);

	if (m_impulseFreqBuffer)
		fftwf_free(m_impulseFreqBuffer);

	if (m_inputFreqBuffer)
		fftwf_free(m_inputFreqBuffer);

	if (m_sumFreqBuffer)
		fftwf_free(m_sumFreqBuffer);
}

void ConvolutionEngine::updateSizes()
{
	m_convSize = nextPowerOfTwo(m_blockSize + m_responseSize - 1);
	configure();
}

void ConvolutionEngine::configure()
{
	if (m_isConfigured)
		cleanup();

	fftwf_complex* freqBuffer = (fftwf_complex*)fftwf_malloc(m_convSize * sizeof(fftwf_complex));
	float* timeBuffer = new float[m_convSize];

	m_r2cPlan = fftwf_plan_dft_r2c_1d(m_convSize, timeBuffer, freqBuffer, FFTW_ESTIMATE);
	m_c2rPlan = fftwf_plan_dft_c2r_1d(m_convSize, freqBuffer, timeBuffer, FFTW_ESTIMATE);

	fftwf_free(freqBuffer);
	delete[] timeBuffer;

	m_impulseFreqBuffer = (fftwf_complex*)fftwf_malloc(m_convSize * sizeof(fftwf_complex));
	m_inputFreqBuffer = (fftwf_complex*)fftwf_malloc(m_convSize * sizeof(fftwf_complex));
	m_sumFreqBuffer = (fftwf_complex*)fftwf_malloc(m_convSize * sizeof(fftwf_complex));

	m_isConfigured = true;
}
