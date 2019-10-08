#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AmbixLoader
{
public:
	AmbixLoader(const File& file);

	void getSourcePositions(std::vector<float>& azi, std::vector<float>& ele);
	void getDecodeMatrix(std::vector<float>& decodeMatrix);
	
	int getNumHrirs();
	void getHrir(int index, AudioBuffer<float>& hrirs);

private:
	void parseFile(const File& file);
	void parseDirection(const String& filename);

	std::vector<AudioBuffer<float>> m_hrirs;
	std::vector<float> m_azi;
	std::vector<float> m_ele;
	std::vector<float> m_decodeMatrix;
};
