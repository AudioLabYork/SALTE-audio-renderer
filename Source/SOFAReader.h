#pragma once

#include "SOFA.h"
#include "SOFAString.h"
#include "ncDim.h"
#include "ncVar.h"

class SOFAReader
{
public:
	SOFAReader(const std::string& filename);

	void displayInformation();
	void getResponseForSpeakerPosition(std::vector<float>& response, float theta, float phi);

	void getImpulseData(std::vector<float>& impulses);
	void getSourcePositions(std::vector<float>& azimuths, std::vector<float>& elevations);
	
	std::size_t getNumImpulses();
	std::size_t getNumImpulseChannels();
	std::size_t getNumImpulseSamples();

private:
	// file handle
	std::unique_ptr<sofa::NetCDFFile> loadedFile;
};
