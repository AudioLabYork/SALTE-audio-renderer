#include "DualBandFilter.h"

DualBandFilter::DualBandFilter() :	m_sampleRate(0),
									m_ambisonicOrder(1)
{

}

DualBandFilter::~DualBandFilter()
{
	lowPassFilterArray.clear();
	highPassFilterArray.clear();
}

void DualBandFilter::init(double sampleRate, int order)
{
	if (order < 1 || order > 7)
		return;

	m_sampleRate = sampleRate;
	m_ambisonicOrder = order;

	// Calculate and set dual band filters coefficients
	setFilterCoeffs();
}


void DualBandFilter::process(AudioSampleBuffer& buffer)
{
	if (m_sampleRate == 0)
		return;

	// get the buffer length and duplicate buffer for dual band processing
	int bufferLength = buffer.getNumSamples();
	lowPassedBuffer.makeCopyOf(buffer);
	highPassedBuffer.makeCopyOf(buffer);

	int numberOfChannelsProcessed = (int)pow(m_ambisonicOrder + 1, 2);

	for (int ch = 0; ch < numberOfChannelsProcessed && numberOfChannelsProcessed <= buffer.getNumChannels(); ++ch)
	{
		lowPassFilterArray[ch]->processSamples(lowPassedBuffer.getWritePointer(ch), bufferLength);
		highPassFilterArray[ch]->processSamples(highPassedBuffer.getWritePointer(ch), bufferLength);

		// Get the Ambisonic order of the current channel
		int currentOrder = floor(sqrt(ch));

		// Apply gain correction and combine two bands
		FloatVectorOperations::multiply(highPassedBuffer.getWritePointer(ch), coeffsGainCorrected[m_ambisonicOrder - 1][currentOrder], bufferLength);
		FloatVectorOperations::add(buffer.getWritePointer(ch), lowPassedBuffer.getReadPointer(ch), highPassedBuffer.getReadPointer(ch), bufferLength);
	}
}

void DualBandFilter::setFilterCoeffs()
{
	double cutoff = xOverFreqs[m_ambisonicOrder - 1];
	double k, k2, a0, denominator, a1, a2, bLp0, bLp1, bLp2, bHp0, bHp1, bHp2;
	k = tan((MathConstants<double>::pi * cutoff) / (m_sampleRate));
	k2 = 2 * k;
	a0 = 1;
	denominator = pow(k, 2) + k2 + 1;
	a1 = (2 * (pow(k, 2) - 1)) / denominator;
	a2 = (pow(k, 2) - k2 + 1) / denominator;
	bLp0 = pow(k, 2) / denominator;
	bLp1 = 2 * bLp0;
	bLp2 = bLp0;
	bHp0 = 1 / denominator;
	bHp1 = -2 * bHp0;
	bHp2 = bHp0;

	int numberOfChannelsProcessed = (int)pow(m_ambisonicOrder + 1, 2);

	lowPassFilterArray.clear();
	highPassFilterArray.clear();

	for (int i = 0; i < numberOfChannelsProcessed; ++i)
	{
		lowPassFilterArray.add(new IIRFilter);
		highPassFilterArray.add(new IIRFilter);
		lowPassFilterArray[i]->setCoefficients(IIRCoefficients(bLp0, bLp1, bLp2, a0, a1, a2));
		highPassFilterArray[i]->setCoefficients(IIRCoefficients(bHp0, bHp1, bHp2, a0, a1, a2));
	}
}
