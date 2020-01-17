#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AmbixLoader
{
public:
	AmbixLoader(const File& file);

	void getSourcePositions(std::vector<float>& azi, std::vector<float>& ele);
	void getDecodeMatrix(std::vector<float>& decodeMatrix);
	
	int getAmbiOrder();
	int getNumAmbiChans();
	int getNumLsChans();

	int getNumHrirs();
	void getHrir(int index, AudioBuffer<float>& hrirs);

private:
	void parseFile(const File& file);
	void parseDirection(const String& filename);

	int m_order;
	int m_numAmbiChans;
	int m_numLsChans;

	std::vector<AudioBuffer<float>> m_hrirs;
	std::vector<float> m_azi;
	std::vector<float> m_ele;
	std::vector<float> m_decodeMatrix;
};
