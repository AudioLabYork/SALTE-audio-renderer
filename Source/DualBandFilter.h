#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>

using std::vector;

class DualBandFilter
{
public:
	DualBandFilter();
	~DualBandFilter();

	void init(double sampleRate, int order);
	void process(AudioSampleBuffer& buffer);

	int getCurrentOrder()
	{
		return m_ambisonicOrder;
	}

	double getCurrentXFreq()
	{
		return xOverFreqs[m_ambisonicOrder - 1];
	}

private:
	double m_sampleRate;
	int m_ambisonicOrder;

	OwnedArray<IIRFilter> lowPassFilterArray, highPassFilterArray;
	AudioBuffer<float> lowPassedBuffer, highPassedBuffer;

	Array<double> xOverFreqs = { 690, 1250, 1831, 2423, 3022, 3605, 4188 };
	vector<vector<double>> coeffsGainCorrected = {
		{- 1.41779401895169, - 0.814424156449370,	0,	0,	0,	0,	0,	0},
		{- 1.58304078061353, - 1.22523496734222, - 0.630932597243196,	0,	0,	0,	0,	0},
		{- 1.66921560486095, - 1.43711245808576, - 1.02131681075692, - 0.507430850075629,	0,	0,	0,	0},
		{- 1.72156214479365, - 1.55991931065421, - 1.25939936643676, - 0.861971185647732, - 0.422267019161326,	0,	0,	0},
		{- 1.75655318309549, - 1.63786700329269, - 1.41252366399666, - 1.10322658466554, - 0.740804516591291, - 0.360768898676049,	0,	0},
		{- 1.78152322701846, - 1.69081543801370, - 1.51632755776978, - 1.27132709899040, - 0.974298005642832, - 0.647381731665108, - 0.314521173194364,	0},
		{- 1.80020698700072, - 1.72868825517082, - 1.58991274225291, - 1.39212215133700, - 1.14699316066438, - 0.868867903803047, - 0.573813056127803, - 0.278572581486516}
	};

	void setFilterCoeffs();
};
