#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class OutputRouting :	public Component,
						public Button::Listener
{
public:
	OutputRouting();
	~OutputRouting();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
	void processBlock(AudioBuffer<float>& buffer);
	void releaseResources();

	void buttonClicked(Button* buttonClicked) override;

	void paint(Graphics& g);
	void resized();

	bool m_shouldBeVisible = false;

	String getRoutingFilePath();
	String getCalibrationFilePath();

	void loadRoutingFile(String chosenFilePath);
	void loadCalibrationFile(String chosenFilePath);


private:

	Array <int> routingMatrix;
	Array <double> calibrationGains;

	String routingFilePath, calibrationFilePath;

	TextButton browseRoutFile, browseCalFile;
	TextButton initRouting, initCalibration;
	TextButton closeWindowBtn;


	void initRoutingMatrix();
	void initCalibrationMatrix();
	void browseForRoutingFile();
	void browseForCalibrationFile();
};
