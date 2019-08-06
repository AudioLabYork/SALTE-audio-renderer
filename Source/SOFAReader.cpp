#include "SOFAReader.h"

SOFAReader::SOFAReader(const std::string& filename)
{
	loadedFile = std::make_unique<sofa::NetCDFFile>(filename);
}

void SOFAReader::displayInformation()
{
	std::vector<std::string> attributeNames;
	loadedFile->GetAllAttributesNames(attributeNames);

	std::cout << "\nGlobal Attributes:\n";

	for (auto& name : attributeNames)
	{
		const std::string value = loadedFile->GetAttributeValueAsString(name);
		std::cout << sofa::String::PadWith(name) << " = " << value << "\n";
	}

	std::vector<std::string> dimensionNames;
	loadedFile->GetAllDimensionsNames(dimensionNames);

	std::cout << "\nDimensions:\n";

	for (auto& name : dimensionNames)
	{
		const std::size_t dim = loadedFile->GetDimension(name);
		std::cout << name << " = " << dim << "\n";
	}

	std::vector<std::string> variableNames;
	loadedFile->GetAllVariablesNames(variableNames);

	std::cout << "\nVariables:\n";

	for (auto& name : variableNames)
	{
		const std::string typeName = loadedFile->GetVariableTypeName(name);
		const std::string dimsNames = loadedFile->GetVariableDimensionsNamesAsString(name);
		const std::string dims = loadedFile->GetVariableDimensionsAsString(name);

		std::cout << name << "\n";
		std::cout << sofa::String::PadWith("Datatype: ")	<< typeName << "\n";
		std::cout << sofa::String::PadWith("Dimensions: ")	<< dimsNames << "\n";
		std::cout << sofa::String::PadWith("Size: ")		<< dims << "\n";

		std::vector<std::string> variableAttributeNames;
		std::vector<std::string> variableAttributeValues;

		loadedFile->GetVariablesAttributes(variableAttributeNames, variableAttributeValues, name);

		if (variableAttributeNames.size() > 0)
		{
			std::cout << sofa::String::PadWith("Attributes: ") << dims << "\n";

			for (std::size_t j = 0; j < variableAttributeNames.size(); j++)
			{
				std::cout << sofa::String::PadWith(variableAttributeNames[j]) << " = " << variableAttributeValues[j] << "\n";
			}
		}

		std::cout << "\n";
	}

	//std::vector<double> delay; // broadband delay in units of N (ie. time axis of FIR)
	//std::vector<double> impulseResponse; // impulse response along the time axis
	//std::vector<double> sampleRate; // sample rate of the impulse response and delay
	//
	//file->GetValues(delay, "Data.Delay");
	//file->GetValues(impulseResponse, "Data.IR");
	//file->GetValues(sampleRate, "Data.SamplingRate");
	//
	//std::vector<double> sourcePosition;
	//
	//file->GetValues(sourcePosition, "SourcePosition");
}

bool SOFAReader::getResponseForSpeakerPosition(std::vector<float>& response, float theta, float phi)
{
	std::size_t N = loadedFile->GetDimension("N"); // number of data samples in a measurement
	std::size_t R = loadedFile->GetDimension("R"); // number of receivers

	std::vector<double> sourcePositions;
	std::vector<double> impulseResponse;

	loadedFile->GetValues(sourcePositions, "SourcePosition");
	loadedFile->GetValues(impulseResponse, "Data.IR");

	response.clear();

	int index = 0;

	for (std::size_t j = 0; j < sourcePositions.size(); j += 3)
	{
		if (theta == sourcePositions[j] && phi == sourcePositions[j + 1])
		{
			//std::cout << "[" << index << "] " << "theta: " << sourcePositions[j] << "\tphi: " << sourcePositions[j + 1] << "\tdis: " << sourcePositions[j + 2] << "\n";

			std::vector<double>::const_iterator first = impulseResponse.begin() + (index * N * R);
			std::vector<double>::const_iterator last = impulseResponse.begin() + (index * N * R) + (N * R);
			
			response.insert(response.begin(), first, last);
			return true;
		}

		index++;
	}

	return false;
}

void SOFAReader::getImpulseData(std::vector<float>& impulses)
{
	std::vector<double> vals;
	loadedFile->GetValues(vals, "Data.IR");

	impulses.clear();

	for (auto& v : vals)
	{
		impulses.push_back(static_cast<float>(v));
	}
}

void SOFAReader::getSourcePositions(std::vector<float>& azimuths, std::vector<float>& elevations)
{
	std::vector<double> sourcePositions;
	loadedFile->GetValues(sourcePositions, "SourcePosition");

	if (sourcePositions.size() > 0)
	{
		for (int i = 0; i < sourcePositions.size(); i += 3)
		{
			azimuths.push_back(static_cast<float>(sourcePositions[i]));
			// there could be an issue here with the + 1 is they are not full groups of 3
			elevations.push_back(static_cast<float>(sourcePositions[i + 1]));
		}
	}
}

float SOFAReader::getSampleRate()
{
	double sampleRate = 0.0;
	loadedFile->GetValues(&sampleRate, 0, 0, "Data.SamplingRate");
	return static_cast<float>(sampleRate);
}

std::size_t SOFAReader::getNumImpulses()
{
	return loadedFile->GetDimension("M"); // number of measurements
}

std::size_t SOFAReader::getNumImpulseChannels()
{
	return loadedFile->GetDimension("R"); // number of receivers
}

std::size_t SOFAReader::getNumImpulseSamples()
{
	return loadedFile->GetDimension("N"); // number of samples describing a single measurement
}
