#include "OutputRouting.h"

OutputRouting::OutputRouting()
{
	browseRoutFile.setButtonText("Load Routing File");
	browseRoutFile.addListener(this);
	addAndMakeVisible(browseRoutFile);

	browseCalFile.setButtonText("Load Calibration File");
	browseCalFile.addListener(this);
	addAndMakeVisible(browseCalFile);

	initRouting.setButtonText("Init Routing");
	initRouting.addListener(this);
	addAndMakeVisible(initRouting);

	initCalibration.setButtonText("Init Calibration");
	initCalibration.addListener(this);
	addAndMakeVisible(initCalibration);

	closeWindowBtn.setButtonText("Close");
	closeWindowBtn.onClick = [this] { setVisible(false); m_shouldBeVisible = false; };
	addAndMakeVisible(closeWindowBtn);

	setSize(480, 680);
}

OutputRouting::~OutputRouting()
{
}

void OutputRouting::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{

}

void OutputRouting::processBlock(AudioBuffer<float>& buffer)
{
	auto totalNumInputChannels = 64;
	auto totalNumOutputChannels = 64;

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	AudioBuffer<float> duplicatedBuffer;
	duplicatedBuffer.makeCopyOf(buffer);

	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		if (routingMatrix.contains(channel))
		{
			buffer.copyFrom(channel, 0, duplicatedBuffer, routingMatrix.indexOf(channel), 0, duplicatedBuffer.getNumSamples());
		}
		else
		{
			buffer.clear(channel, 0, buffer.getNumSamples());
		}

		double logGain = calibrationGains[channel];
		double linGain = pow(10, logGain / 20.0);
		buffer.applyGain(channel, 0, buffer.getNumSamples(), linGain);
	}
}

void OutputRouting::releaseResources()
{
}

void OutputRouting::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

	g.setColour(Colours::white);
	g.setFont(12.0f);

	const int marginLeft = 70;
	const int marginTop = 120;

	for (int i = 0; i < 64; ++i)
	{
		g.setColour(Colours::green);
		if (i % 10 == 0) g.drawText("In", 20 + (i % 10) * 40, marginTop + (floor(i / 10) * 80), 30, 25, Justification::centredLeft);
		if (routingMatrix.contains(i))
		{
			String	inputChIndex = String(routingMatrix.indexOf(i) + 1);
			g.drawText(inputChIndex, marginLeft + (i % 10) * 40, marginTop + (floor(i / 10) * 80), 30, 25, Justification::centred);
		}

		g.setColour(Colours::yellow);
		if (i % 10 == 0) g.drawText("Out", 20 + (i % 10) * 40, marginTop + 20 + (floor(i / 10) * 80), 30, 25, Justification::centredLeft);
		String speakerNumber = String(i + 1);
		g.drawText(speakerNumber, marginLeft + (i % 10) * 40, marginTop + 20 + (floor(i / 10) * 80), 30, 25, Justification::centred);

		g.setColour(Colours::red);
		if (i % 10 == 0) g.drawText("Gain", 20 + (i % 10) * 40, marginTop + 40 + (floor(i / 10) * 80), 30, 25, Justification::centredLeft);
		String calGainValue = String(calibrationGains[i], 2);
		g.drawText(calGainValue, marginLeft + (i % 10) * 40, marginTop + 40 + (floor(i / 10) * 80), 30, 25, Justification::centred);
	}

	g.setColour(Colours::white);
	g.drawText(routingFilePath, 10, 35, 420, 25, Justification::centredLeft);
	g.drawText(calibrationFilePath, 10, 90, 420, 25, Justification::centredLeft);
}

void OutputRouting::resized()
{
	browseRoutFile.setBounds(160, 10, 160, 25);
	browseCalFile.setBounds(160, 65, 160, 25);
	initRouting.setBounds(20, 10, 120, 25);
	initCalibration.setBounds(20, 65, 120, 25);
	closeWindowBtn.setBounds(getWidth() - 120, getHeight() - 45, 100, 25);
}

void OutputRouting::buttonClicked(Button* buttonClicked)
{
	if (buttonClicked == &browseRoutFile)
	{
		browseForRoutingFile();
	}
	else if (buttonClicked == &browseCalFile)
	{
		browseForCalibrationFile();
	}
	else if (buttonClicked == &initRouting)
	{
		initRoutingMatrix();
	}
	else if (buttonClicked == &initCalibration)
	{
		initCalibrationMatrix();
	}
	repaint();
}

String OutputRouting::getRoutingFilePath()
{
	return routingFilePath;
}

String OutputRouting::getCalibrationFilePath()
{
	return calibrationFilePath;
}


void OutputRouting::browseForRoutingFile()
{
#if JUCE_MODAL_LOOPS_PERMITTED
	const bool useNativeVersion = true;
	FileChooser fc("Choose a file to open...",
		File::getCurrentWorkingDirectory(),
		"*.txt",
		useNativeVersion);

	if (fc.browseForFileToOpen())
	{
		File chosenFile = fc.getResult();
		loadRoutingFile(chosenFile.getFullPathName());
	}
#endif
}

void OutputRouting::loadRoutingFile(String chosenFilePath)
{
	File chosenFile = File(chosenFilePath);
	String dataToLoad = chosenFile.loadFileAsString();
	StringArray loadedData;
	loadedData.clear();
	loadedData.addLines(dataToLoad);

	if (loadedData[0].startsWith("#RIGRTG#"))
	{
		int inputChCounter = 0;
		for (int i = 2; i < loadedData.size(); i++)
		{
			StringArray tokens;
			tokens.addTokens(loadedData[i], "\t", "\"");
			routingMatrix.set(inputChCounter, tokens[1].getIntValue() - 1);
			inputChCounter++;
		}
		routingMatrix.resize(inputChCounter);
		routingFilePath = chosenFile.getFullPathName();
	}
	else
	{
		initRoutingMatrix();
	}
}

void OutputRouting::initRoutingMatrix()
{
	const int size = 64;
	routingMatrix.resize(size);
	for (int i = 0; i < size; ++i) routingMatrix.set(i, i);

	routingFilePath.clear();
}

void OutputRouting::initCalibrationMatrix()
{
	const int size = 64;
	calibrationGains.resize(size);
	for (int i = 0; i < size; ++i) calibrationGains.set(i, 0.0f);

	calibrationFilePath.clear();
}

void OutputRouting::browseForCalibrationFile()
{
#if JUCE_MODAL_LOOPS_PERMITTED
	const bool useNativeVersion = true;
	FileChooser fc("Choose a file to open...",
		File::getCurrentWorkingDirectory(),
		"*.txt",
		useNativeVersion);

	if (fc.browseForFileToOpen())
	{
		File chosenFile = fc.getResult();
		loadCalibrationFile(chosenFile.getFullPathName());
	}
#endif
}

void OutputRouting::loadCalibrationFile(String chosenFilePath)
{
	File chosenFile = File(chosenFilePath);
	String dataToLoad = chosenFile.loadFileAsString();
	StringArray loadedData;
	loadedData.clear();
	loadedData.addLines(dataToLoad);

	if (loadedData[0].startsWith("#RIGCAL#"))
	{
		for (int i = 2; i < loadedData.size(); i++)
		{
			StringArray tokens;
			tokens.addTokens(loadedData[i], "\t", "\"");
			calibrationGains.set(i, tokens[1].getFloatValue());
		}
		calibrationFilePath = chosenFile.getFullPathName();
	}
	else
	{
		initCalibrationMatrix();
	}
}