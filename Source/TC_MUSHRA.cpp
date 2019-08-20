#include "TC_MUSHRA.h"

//==============================================================================
MushraComponent::MushraComponent()
{
}

MushraComponent::~MushraComponent()
{
}

//==============================================================================
void MushraComponent::paint (Graphics& g)
{
	// BACKGROUND
	//g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    g.fillAll(Colours::gainsboro);

	// RECTANGULAR OUTLINE
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

    g.setFont (Font (16.0f));
    g.setColour (Colours::black);
	g.drawText("MUSHRA Test Mode", 25, 25, 200, 25, Justification::centredLeft, true);
	g.setFont(Font(14.0f));
    g.drawText("Trial: " + String(currentTrialIndex + 1) + " of " + String(regionArray.size()), 390, 680, 200, 20, Justification::centredLeft, true);
	g.drawText("Region: " + String(currentRegion + 1) + " of " + String(regionArray.size()), 390, 700, 200, 20, Justification::centredLeft, true);
	g.drawText("Sample: " + String(currentTrack + 1) + " of " + String(numberOfSamplesPerRegion), 390, 720, 200, 20, Justification::centredLeft, true);
    g.drawText("Start: " + String(regionArray[currentRegion]->dawStartTime), 545, 700, 200, 20, Justification::centredLeft, true);
    g.drawText("Stop: " + String(regionArray[currentRegion]->dawStopTime), 545, 720, 200, 20, Justification::centredLeft, true);


	int linesStartX = 125;
	int linesEndX = 925;
	int linesStartY = 0.06 * getHeight();
	int linesEndY = 0.66 * getHeight();
	int linesYinterval = (linesEndY - linesStartY) / 5;

	Line<float> line1(Point<float>(linesStartX, linesStartY), Point<float>(linesEndX, linesStartY));
	Line<float> line2(Point<float>(linesStartX, linesStartY + linesYinterval * 1), Point<float>(linesEndX, linesStartY + linesYinterval * 1));
	Line<float> line3(Point<float>(linesStartX, linesStartY + linesYinterval * 2), Point<float>(linesEndX, linesStartY + linesYinterval * 2));
	Line<float> line4(Point<float>(linesStartX, linesStartY + linesYinterval * 3), Point<float>(linesEndX, linesStartY + linesYinterval * 3));
	Line<float> line5(Point<float>(linesStartX, linesStartY + linesYinterval * 4), Point<float>(linesEndX, linesStartY + linesYinterval * 4));
	Line<float> line6(Point<float>(linesStartX, linesEndY), Point<float>(linesEndX, linesEndY));

	float dashPattern[2];
	dashPattern[0] = 4.0;
	dashPattern[1] = 8.0;
	g.setColour(Colours::dimgrey);

	//g.drawDashedLine(line1, dashPattern, 2, 1.0f);
	g.drawDashedLine(line2, dashPattern, 2, 1.0f);
	g.drawDashedLine(line3, dashPattern, 2, 1.0f);
	g.drawDashedLine(line4, dashPattern, 2, 1.0f);
	g.drawDashedLine(line5, dashPattern, 2, 1.0f);
	//g.drawDashedLine(line6, dashPattern, 2, 1.0f);

	int textStartX = linesStartX - 60;
	int textWidth = 80;
	g.drawText("Excellent", textStartX, linesStartY, textWidth, linesYinterval, Justification::centredRight, true);
	g.drawText("Good", textStartX, linesStartY + linesYinterval * 1, textWidth, linesYinterval, Justification::centredRight, true);
	g.drawText("Fair", textStartX, linesStartY + linesYinterval * 2, textWidth, linesYinterval, Justification::centredRight, true);
	g.drawText("Poor", textStartX, linesStartY + linesYinterval * 3, textWidth, linesYinterval, Justification::centredRight, true);
	g.drawText("Bad", textStartX, linesStartY + linesYinterval * 4, textWidth, linesYinterval, Justification::centredRight, true);

}

void MushraComponent::createGui()
{
	int numberOfRegions = 4;
	for (int i = 0; i < numberOfRegions; ++i)
	{
		regionArray.add(new SampleRegion());
		regionArray[i]->dawStartTime = 0.0f; //markerTimeArray[i * 2];         // 0 2 4 6
		regionArray[i]->dawStopTime = 5.0f;  //markerTimeArray[(i * 2) + 1];    // 1 3 5 7
		regionArray[i]->calculateStartEndTimes();
	}

	int numberOfSamplesPerRegion = 8;
	numberOfSamplesPerRegion = numberOfSamplesPerRegion;
	
	
	// scores array
	scoresArray.resize(regionArray.size());
	for (int i = 0; i < regionArray.size(); ++i) scoresArray[i].resize(numberOfSamplesPerRegion);

	// Put some values in
	//scoresArray[1][2] = 6.0;
	//scoresArray[3][1] = 5.5;

	// trial randomization array
	trialRandomizationArray.resize(regionArray.size());
	for (int i = 0; i < regionArray.size(); ++i) trialRandomizationArray[i] = i;
	std::random_device seed;
	std::mt19937 rng(seed());
	std::shuffle(trialRandomizationArray.begin(), trialRandomizationArray.end(), rng);
	currentRegion = trialRandomizationArray[currentTrialIndex]; // set the first region according to the randomized order
	
	// sample randomization array
	sampleRandomizationArray.resize(regionArray.size());
	for (int i = 0; i < regionArray.size(); ++i)
	{
		sampleRandomizationArray[i].resize(numberOfSamplesPerRegion);
		for (int j = 0; j < numberOfSamplesPerRegion; ++j) sampleRandomizationArray[i][j] = j;
        
        std::random_device seed;
        std::mt19937 rng(seed());
        std::shuffle(sampleRandomizationArray[i].begin(), sampleRandomizationArray[i].end(), rng);
	}
	
	for (int i = 0; i < numberOfSamplesPerRegion; ++i)
	{
		rateSampleSliderArray.add(new Slider());
		rateSampleSliderArray[i]->getProperties().set("rateSampleSlider", true);
		rateSampleSliderArray[i]->getProperties().set("sliderIndex", i);
        rateSampleSliderArray[i]->setSliderStyle(Slider::LinearBarVertical);
        rateSampleSliderArray[i]->setTextBoxIsEditable(false);
        rateSampleSliderArray[i]->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
		rateSampleSliderArray[i]->setRange(0, 100, 1);
		rateSampleSliderArray[i]->setValue(0);
		rateSampleSliderArray[i]->addListener(this);
		addAndMakeVisible(rateSampleSliderArray[i]);
	}

    for (int i = 0; i < numberOfSamplesPerRegion; ++i)
    {
        rateSampleSliderLabelArray.add(new Label());
        rateSampleSliderLabelArray[i]->setText(String(scoresArray[currentRegion][i],0), dontSendNotification);
        rateSampleSliderLabelArray[i]->setJustificationType(Justification::centred);
		rateSampleSliderLabelArray[i]->setColour(Label::textColourId, Colours::black);
        addAndMakeVisible(rateSampleSliderLabelArray[i]);
    }
    
	StringArray playButtonAlphabet = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
                                    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};
	for (int i = 0; i < numberOfSamplesPerRegion; ++i)
	{
		playSampleButtonArray.add(new TextButton());
		playSampleButtonArray[i]->getProperties().set("playSampleButton", true);
		playSampleButtonArray[i]->getProperties().set("buttonIndex", i);
		if (i < playButtonAlphabet.size()) playSampleButtonArray[i]->setButtonText(playButtonAlphabet[i]);
		playSampleButtonArray[i]->addListener(this);
		addAndMakeVisible(playSampleButtonArray[i]);
	}

	playReference.setButtonText("Reference");
	playReference.addListener(this);
	if (isReferenceButtonVisible) addAndMakeVisible(playReference);

	loopSlider.setSliderStyle(Slider::ThreeValueHorizontal);
	loopSlider.setRange(0, 1);
	loopSlider.setMinAndMaxValues(0, 1);
	loopSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
	loopSlider.addListener(this);
	addAndMakeVisible(loopSlider);

	loopTB.setButtonText("Loop");
	loopTB.setToggleState(loop, dontSendNotification);
	loopTB.addListener(this);
	loopTB.setColour(ToggleButton::textColourId, Colours::black);
	loopTB.setColour(ToggleButton::tickColourId, Colours::black);
	loopTB.setColour(ToggleButton::tickDisabledColourId, Colours::black);
	addAndMakeVisible(loopTB);

	stopPlaybackB.setButtonText("Stop");
	stopPlaybackB.addListener(this);
	addAndMakeVisible(stopPlaybackB);

	goToPrevious.setButtonText("Previous Trial");
	goToPrevious.addListener(this);
	addAndMakeVisible(goToPrevious);

	goToNext.setButtonText("Next Trial");
	goToNext.addListener(this);
	addAndMakeVisible(goToNext);


	dawTimeLabel.setColour(Label::textColourId, Colours::black);
	sampleTimeLabel.setColour(Label::textColourId, Colours::black);
	addAndMakeVisible(dawTimeLabel);
	addAndMakeVisible(sampleTimeLabel);

	updateTransportSlider(true);
	updateClientTransportSlider();
	updateClientRatingSliders();

	resized();
}

void MushraComponent::connectOsc(String dawIp, String clientIp, int dawTxPort, int dawRxPort, int clientTxPort, int clientRxPort)
{
	// DAW
	//dawTx.connectSender(dawIp, dawTxPort);
	//dawRx.connectReceiver(dawRxPort);
	//dawRx.addListener(this);

	//// CLIENT
	//clientTx.connectSender(clientIp, clientTxPort);
	//clientRx.connectReceiver(clientRxPort);
	//clientRx.addListener(this);

	clientRxPortAtHost = clientRxPort;

	// DAW state
	stopPlayback();
}

void MushraComponent::resized()
{
	auto rateRectWidth = 0.8 * getWidth();
	auto rateRectHeight = 0.6 * getHeight();
	
	auto sliderSpacing = rateRectWidth / (numberOfSamplesPerRegion + 1);
	
	auto sliderWidth = 50;
	auto sliderHeight = rateRectHeight;
	
	auto buttonWidth = 65;
	auto buttonHeight = buttonWidth;
	auto labelHeight = 0.5 * buttonWidth;


	auto sliderPositionX = (getWidth() - rateRectWidth) / 2 + sliderWidth / 2;
	auto sliderPositionY = 0.06 * getHeight() + sliderHeight / 2;
	auto labelPositionY = sliderPositionY + sliderHeight / 2 + labelHeight * 0.5;
	auto buttonPositionY = sliderPositionY + sliderHeight / 2 + buttonHeight;

	for (int i = 0; i < numberOfSamplesPerRegion; ++i)
	{
		rateSampleSliderArray[i]->setSize(sliderWidth, sliderHeight);
        rateSampleSliderLabelArray[i]->setSize(buttonWidth, labelHeight);
		playSampleButtonArray[i]->setSize(buttonWidth, buttonHeight);

		rateSampleSliderArray[i]->setCentrePosition(sliderPositionX + (i+1) * sliderSpacing, sliderPositionY);
		rateSampleSliderLabelArray[i]->setCentrePosition(sliderPositionX + (i+1) * sliderSpacing, labelPositionY);
		playSampleButtonArray[i]->setCentrePosition(sliderPositionX + (i+1) * sliderSpacing, buttonPositionY);
	}

	playReference.setBounds(25, buttonPositionY - buttonHeight / 2, 80, buttonHeight);
	stopPlaybackB.setBounds(25, buttonPositionY + buttonHeight, 80, buttonHeight);
	loopSlider.setBounds(sliderPositionX, buttonPositionY + buttonHeight, rateRectWidth, labelHeight);

	loopTB.setBounds(0.8 * getWidth(), 0.875 * getHeight(), 80, labelHeight);
	goToPrevious.setBounds(sliderPositionX, buttonPositionY + 2 * buttonHeight, 100, labelHeight);
	goToNext.setBounds(sliderPositionX + 125, buttonPositionY + 2 * buttonHeight, 100, labelHeight);

	dawTimeLabel.setBounds(700, 700, 200, 20);
	sampleTimeLabel.setBounds(700, 720, 200, 20);
}

void MushraComponent::buttonClicked (Button* buttonThatWasClicked)
{
	bool playSampleButtonState = buttonThatWasClicked->getProperties()["playSampleButton"];
    if (playSampleButtonState)
	{
		int buttonIndex = buttonThatWasClicked->getProperties()["buttonIndex"];
		playSample(buttonIndex, randomizeSamples);
	}
	else if (buttonThatWasClicked == &playReference)
	{
		playSample(0, false);
	}
	else if (buttonThatWasClicked == &stopPlaybackB)
	{
        stopPlayback();
	}
	else if (buttonThatWasClicked == &loopTB)
	{
		loop = loopTB.getToggleState();
	}
	else if (buttonThatWasClicked == &goToNext)
	{
		if (currentTrialIndex < regionArray.size() - 1)
        {
            currentTrialIndex++;
			currentRegion = trialRandomizationArray[currentTrialIndex];
            stopPlayback();
            updateTransportSlider(true);
			updateClientTransportSlider();
			updateRatingSliders();
			updateClientRatingSliders();
        }

	}
	else if (buttonThatWasClicked == &goToPrevious)
	{
		if (currentTrialIndex > 0)
        {
			currentTrialIndex--;
			currentRegion = trialRandomizationArray[currentTrialIndex];
			stopPlayback();
            updateTransportSlider(true);
			updateClientTransportSlider();
            updateRatingSliders();
			updateClientRatingSliders();
        }
	}
	repaint();
}

void MushraComponent::sliderValueChanged(Slider *sliderThatWasChanged)
{
	bool rateSampleSliderChanged = sliderThatWasChanged->getProperties()["rateSampleSlider"];

	if (rateSampleSliderChanged)
	{
		int sliderIndex = sliderThatWasChanged->getProperties()["sliderIndex"];
        int trackNum = sampleRandomizationArray[currentRegion][sliderIndex];
		scoresArray[currentRegion][trackNum] = sliderThatWasChanged->getValue();
		
        for (int i = 0; i < numberOfSamplesPerRegion; ++i)
        {
            int trackNum = sampleRandomizationArray[currentRegion][i];
            rateSampleSliderLabelArray[i]->setText(String(scoresArray[currentRegion][trackNum],0), dontSendNotification);
        }
	}
	else if (sliderThatWasChanged == &loopSlider)
	{
		float regionLength = regionArray[currentRegion]->getRegionLength();
		regionArray[currentRegion]->loopStartOffset = loopSlider.getMinValue();
		regionArray[currentRegion]->loopEndOffset = regionLength - loopSlider.getMaxValue();
		regionArray[currentRegion]->calculateStartEndTimes();
		updateClientTransportSlider();
	}
}

void MushraComponent::playSample(int track, bool randomize)
{
//    if (randomize) track = sampleRandomizationArray[currentRegion][track];
//
//    if (playback == true)         stopPlayback(); //dawTx.sendOscMessage("/stop");
//    dawTx.sendOscMessage("/time", regionArray[currentRegion]->startTime);
//    if (track != currentTrack)
//    {
//        dawTx.sendOscMessage("/track/" + String(currentTrack + 1) + "/solo", 0);
//    }
//    dawTx.sendOscMessage("/track/" + String(track + 1) + "/solo", 1);
//    dawTx.sendOscMessage("/play");
//    playback = true;
//    currentTrack = track;
}

void MushraComponent::playSampleLoop()
{
//    dawTx.sendOscMessage("/track/" + String(0) + "/mute", 1);
//    dawTx.sendOscMessage("/time", regionArray[currentRegion]->startTime);
//    //Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 100 );
//    dawTx.sendOscMessage("/track/" + String(0) + "/mute", 0);
}

void MushraComponent::stopPlayback()
{
//    dawTx.sendOscMessage("/track/" + String(0) + "/mute", 1);
//    dawTx.sendOscMessage("/stop");
//    currentPosition = regionArray[currentRegion]->startTime;
//    dawTx.sendOscMessage("/time", currentPosition);
//    dawTx.sendOscMessage("/soloreset");
//    //Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 100);
//    dawTx.sendOscMessage("/track/" + String(0) + "/mute", 0);
}

void MushraComponent::oscMessageReceived(const OSCMessage& message)
{
    // DAW
	
	if (message.size() == 1 && message.getAddressPattern() == "/time" && message[0].isFloat32())
    {
        currentPosition = message[0].getFloat32();
        updateTransportSlider(false);
		updateClientTransportSlider();
        
        dawTimeLabel.setText("DAW Time: " + String(currentPosition, 3), NotificationType::dontSendNotification);
		sampleTimeLabel.setText("Sample Time: " + String(currentPosition - regionArray[currentRegion]->startTime, 3), NotificationType::dontSendNotification);
		
        if(currentPosition >= regionArray[currentRegion]->stopTime)
        {
            if(loop) playSampleLoop();
            else stopPlayback();
        }
    }


	// CLIENT
	if (message.size() == 1 && message.getAddressPattern() == "/host/play" && message[0].isInt32())
	{
		playSampleButtonArray[message[0].getInt32()]->triggerClick();
	}
	else if (message.size() == 1 && message.getAddressPattern() == "/host" && message[0].isString())
	{
		if (message[0].getString() == "playref") playReference.triggerClick();
		if (message[0].getString() == "stopplayback") stopPlaybackB.triggerClick();
		if (message[0].getString() == "loopon") loopTB.setToggleState(true, sendNotification);
		if (message[0].getString() == "loopoff") loopTB.setToggleState(false, sendNotification);
		if (message[0].getString() == "gonext") goToNext.triggerClick();
		if (message[0].getString() == "goprev") goToPrevious.triggerClick();
	}
	else if (message.size() == 2 && message.getAddressPattern() == "/host/slider" && message[0].isInt32() && message[1].isFloat32())
	{
		rateSampleSliderArray[message[0].getInt32()]->setValue(message[1].getFloat32());
	}
    else if (message.size() == 4 && message.getAddressPattern() == "/host/loopslider" && message[0].isFloat32() && message[1].isFloat32() && message[2].isFloat32() && message[3].isFloat32())
    {
        regionArray[currentRegion]->loopStartOffset = message[2].getFloat32();
        regionArray[currentRegion]->loopEndOffset = message[0].getFloat32() - message[3].getFloat32();
		regionArray[currentRegion]->calculateStartEndTimes();
        updateTransportSlider(true);
    }
}

void MushraComponent::updateTransportSlider(bool updateLoop)
{
	float regionLength = regionArray[currentRegion]->getRegionLength();
	if (updateLoop)
	{
        loopSlider.setRange(0, regionLength, dontSendNotification);
        loopSlider.setValue(regionArray[currentRegion]->loopStartOffset, dontSendNotification);
		loopSlider.setMinValue(regionArray[currentRegion]->loopStartOffset, dontSendNotification);
		loopSlider.setMaxValue(regionLength - regionArray[currentRegion]->loopEndOffset, dontSendNotification);
	}
	else
	{
		loopSlider.setValue(currentPosition - regionArray[currentRegion]->dawStartTime, dontSendNotification);
	}
}

void MushraComponent::initClientGui()
{
	clientTx.sendOscMessage("/client/hostip", hostIp);
	clientTx.sendOscMessage("/client/hostport", clientRxPortAtHost);
	clientTx.sendOscMessage("/client/nofsamples", numberOfSamplesPerRegion);
	clientTx.sendOscMessage("/client/currenttrial/", (int)currentTrialIndex, (int)regionArray.size());
	clientTx.sendOscMessage("/client/isrefbutton", 1);
	clientTx.sendOscMessage("/client/gui", "create");

}

void MushraComponent::updateClientTransportSlider()
{
//    float regionLength = regionArray[currentRegion]->getRegionLength();
//    clientTx.sendOscMessage("/client/loopslider/", regionLength, currentPosition - regionArray[currentRegion]->dawStartTime, regionArray[currentRegion]->loopStartOffset, regionLength - regionArray[currentRegion]->loopEndOffset);
}

void MushraComponent::updateClientRatingSliders()
{
//    for (int sliderIndex = 0; sliderIndex < rateSampleSliderArray.size(); ++sliderIndex)
//    {
//        clientTx.sendOscMessage("/client/slider/", (int)sliderIndex, (float)rateSampleSliderArray[sliderIndex]->getValue());
//    }
//    clientTx.sendOscMessage("/client/currenttrial/", (int)currentTrialIndex, (int)regionArray.size());
}

void MushraComponent::updateRatingSliders()
{
	for (int i = 0; i < numberOfSamplesPerRegion; ++i)
	{
        int trackNum = sampleRandomizationArray[currentRegion][i];
		rateSampleSliderArray[i]->setValue(scoresArray[currentRegion][trackNum]);
	}
}

void MushraComponent::saveResults()
{
	File chosenFile;
	String dataToSave;

#if JUCE_MODAL_LOOPS_PERMITTED
	FileChooser fc("Choose a file to save...", File::getCurrentWorkingDirectory(),
		"*.txt", true);
	if (fc.browseForFileToSave(true))
	{
		chosenFile = fc.getResult().withFileExtension(".txt");

		dataToSave = String("##### RAW RESULTS #####\n\n");
		for (int i = 0; i < scoresArray.size(); i++)
		{
			for (int j = 0; j < scoresArray[0].size(); j++)
			{
				dataToSave += String(scoresArray[i][j]);
				dataToSave += "\t";
			}
			dataToSave += "\n";
		}

		dataToSave += String("\n");
		dataToSave += String("##### TRIAL RANDOMIZATION ARRAY #####\n\n");
		for (int i = 0; i < scoresArray.size(); i++)
		{
			dataToSave += String(trialRandomizationArray[i]);
			dataToSave += "\t";
		}
		dataToSave += String("\n\n");
		dataToSave += String("##### SAMPLE RANDOMIZATION ARRAY #####\n\n");
		for (int i = 0; i < scoresArray.size(); i++)
		{
			for (int j = 0; j < scoresArray[0].size(); j++)
			{
				dataToSave += String(sampleRandomizationArray[i][j]);
				dataToSave += "\t";
			}
			dataToSave += "\n";
		}

		dataToSave += "\n" + String("##### TRACK NAMES #####\n\n");
		for (int i = 0; i < trackNameArray.size(); i++)
		{
			dataToSave += trackNameArray[i] + "\n";
		}
		dataToSave += "\n" + String("##### DATE AND TIME #####\n\n");
		dataToSave += Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S") + "\n";

		dataToSave += "\n" + String("##### REGIONS #####\n\n");
		for (int i = 0; i < regionArray.size(); i++)
		{
			dataToSave += "Region: " + String(i + 1) + "\n";
			dataToSave += "Beginning: " + String(regionArray[i]->dawStartTime) + "\n";
			dataToSave += "End: " + String(regionArray[i]->dawStopTime) + "\n\n";
		}
		chosenFile.replaceWithText(dataToSave);
	}
#endif
}
