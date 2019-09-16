#include "TC_MUSHRA.h"

MushraComponent::MushraComponent()
	: m_oscTxRx(nullptr)
	, m_player(nullptr)
	, m_renderer(nullptr)
	, leftBorder(20)
	, rightBorder(20)
	, topBorder(20)
	, bottomBorder(20)
{
	prevTrialButton.setButtonText("Previous Trial");
	prevTrialButton.setEnabled(false);
	prevTrialButton.addListener(this);
	addAndMakeVisible(prevTrialButton);

	nextTrialButton.setButtonText("Next Trial");
	nextTrialButton.addListener(this);
	addAndMakeVisible(nextTrialButton);

	endTestButton.setButtonText("End Test");
	endTestButton.setEnabled(false);
	endTestButton.addListener(this);
	addAndMakeVisible(endTestButton);

	selectReferenceButton.setButtonText("Reference");
	selectReferenceButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
	selectReferenceButton.setColour(TextButton::buttonOnColourId, Colours::green);
	selectReferenceButton.addListener(this);
	addChildComponent(selectReferenceButton);
}

MushraComponent::~MushraComponent()
{
}

void MushraComponent::init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRenderer* renderer)
{
	m_oscTxRx = oscTxRx;
	m_oscTxRx->addListener(this);
	
	m_renderer = renderer;
	
	m_player = player;
	m_player->addChangeListener(this);
}

void MushraComponent::reset()
{
	nextTrialButton.setEnabled(true);
	prevTrialButton.setEnabled(false);
	endTestButton.setEnabled(false);
}

void MushraComponent::loadTestSession(TestSession* testSession)
{
	m_testSession = testSession;
	loadTrial(0);
}

void MushraComponent::addListener(Listener* newListener)
{
	mushraTestListeners.add(newListener);
}

void MushraComponent::removeListener(Listener* listener)
{
	mushraTestListeners.remove(listener);
}

void MushraComponent::loadTrial(int trialIndex)
{
	TestTrial* trial = m_testSession->getTrial(trialIndex);
	
	if ((m_testSession == nullptr) || (m_renderer == nullptr) || (m_player == nullptr) || (m_oscTxRx == nullptr) || (trial == nullptr))
	{
		jassertfalse;
		return;
	}

	m_player->unloadFileFromTransport();

	selectReferenceButton.setVisible(trial->isReferencePresent());

	ratingSliderArray.clear();
	ratingReadouts.clear();
	selectConditionButtonArray.clear();

	StringArray selectButtonAlphabet = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
											"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };

	for (int i = 0; i < trial->getNumberOfConditions(); ++i)
	{
		ratingSliderArray.add(new Slider());
		ratingSliderArray[i]->getProperties().set("ratingSlider", true);
		ratingSliderArray[i]->getProperties().set("sliderIndex", i);
		ratingSliderArray[i]->setSliderStyle(Slider::LinearVertical);
		ratingSliderArray[i]->setTextBoxIsEditable(true);
		ratingSliderArray[i]->setRange(0, 100, 1);
		ratingSliderArray[i]->setValue(trial->getCondition(i)->score, dontSendNotification);
		ratingSliderArray[i]->addListener(this);
		addAndMakeVisible(ratingSliderArray[i]);

		ratingReadouts.add(new Label());
		ratingReadouts[i]->setText(String(trial->getCondition(i)->score), dontSendNotification);
		ratingReadouts[i]->setJustificationType(Justification::centred);
		addAndMakeVisible(ratingReadouts[i]);

		selectConditionButtonArray.add(new TextButton());
		selectConditionButtonArray[i]->setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
		selectConditionButtonArray[i]->setColour(TextButton::buttonOnColourId, Colours::coral);
		selectConditionButtonArray[i]->getProperties().set("playSampleButton", true);
		selectConditionButtonArray[i]->getProperties().set("buttonIndex", i);

		if (i < selectButtonAlphabet.size())
			selectConditionButtonArray[i]->setButtonText(selectButtonAlphabet[i]);

		selectConditionButtonArray[i]->addListener(this);
		addAndMakeVisible(selectConditionButtonArray[i]);
	}

	m_player->loop(trial->getLoopingState());
	repaint();
}

void MushraComponent::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

	g.setColour(Colours::white);

	if (m_testSession->getNumberOfTrials() > 0)
	{
		TestTrial* trial = m_testSession->getTrial(m_testSession->getCurrentTrialIndex());

		if (trial == nullptr)
			return;

		g.drawText("Trial " + String(m_testSession->getCurrentTrialIndex() + 1) + " of " + String(m_testSession->getNumberOfTrials()), leftBorder, topBorder, getWidth() - (leftBorder + rightBorder), 25, Justification::centredLeft, true);
		g.drawText(trial->getTrialName(), leftBorder, topBorder + 20, getWidth() - (leftBorder + rightBorder), 25, Justification::centredLeft, true);

		StringArray ratings = trial->getRatingOptions();

		if (!ratings.isEmpty())
		{
			g.setFont(14.0f);

			int textWidth = 0;

			for (int i = 0; i < ratings.size(); ++i)
			{
				int size = g.getCurrentFont().getStringWidth(ratings[i]);
				if (size > textWidth)
					textWidth = size;
			}

			int ySpacer = testArea.getHeight() / ratings.size();

			const int textStartX = leftBorder;
			const int textStartY = testArea.getY();

			for (int i = 0; i < ratings.size(); ++i)
				g.drawText(ratings[i], textStartX, textStartY + ySpacer * i, textWidth, ySpacer, Justification::centredRight, true);

			const int lineXSpacer = 20;
			const int linesStartX = testArea.getX();
			const int linesStartY = testArea.getY();
			const int linesWidth = testArea.getWidth();
			const int numLines = ratings.size() + 1;
			const float dashPattern[2] = { 4.0, 8.0 };

			g.setColour(Colours::ghostwhite);

			for (int i = 0; i < numLines; ++i)
				g.drawDashedLine(Line<float>(Point<float>(linesStartX, linesStartY + ySpacer * i), Point<float>(linesStartX + linesWidth, linesStartY + ySpacer * i)), dashPattern, 2, 1.0f);
		}

		if (trial->getNumberOfConditions() > 0)
		{
			const int sliderSpacing = 0;
			const int sliderWidth = 30;
			const int sliderHeight = testArea.getHeight();
			const int inc = testArea.getWidth() / trial->getNumberOfConditions();
			const int sliderPositionX = testArea.getX();
			const int sliderPositionY = testArea.getY();
			const int buttonHeight = 20;

			for (int i = 0; i < trial->getNumberOfConditions(); ++i)
			{
				ratingSliderArray[i]->setBounds(sliderPositionX + (i * inc), sliderPositionY, sliderWidth, sliderHeight);
				ratingReadouts[i]->setBounds(sliderPositionX + (i * inc), testArea.getBottom() + 10, sliderWidth, buttonHeight);
				selectConditionButtonArray[i]->setBounds(sliderPositionX + (i * inc), testArea.getBottom() + 40, sliderWidth, buttonHeight);
			}
		}
	}
}

void MushraComponent::resized()
{
	testSpace.setBounds(leftBorder, topBorder, getWidth() - (leftBorder + rightBorder), getHeight() - (topBorder + bottomBorder));
	testArea.setBounds(testSpace.getX() + 90, testSpace.getY() + 50, testSpace.getWidth() - 90, testSpace.getHeight() - 180);

	prevTrialButton.setBounds(testSpace.getX(), testSpace.getBottom() - 25, 80, 25);
	nextTrialButton.setBounds(testSpace.getX() + 80 + 10, testSpace.getBottom() - 25, 80, 25);
	endTestButton.setBounds(testSpace.getX() + 160 + 20, testSpace.getBottom() - 25, 80, 25);
	selectReferenceButton.setBounds(testArea.getX(), testSpace.getBottom() - 60, testArea.getWidth(), 25);
}

void MushraComponent::buttonClicked(Button* buttonThatWasClicked)
{
	TestTrial* trial = m_testSession->getTrial(m_testSession->getCurrentTrialIndex());

	for (int i = 0; i < selectConditionButtonArray.size(); ++i)
	{
		if (buttonThatWasClicked == selectConditionButtonArray[i])
		{
			if (trial == nullptr)
				break;

			m_player->pause();

			trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
			
			m_player->loadFile(trial->getCondition(i)->filepath);
			m_player->setGain(trial->getCondition(i)->gain);
			m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());

			m_renderer->setOrder(trial->getCondition(i)->rendereringOrder);
			m_renderer->loadFromAmbixConfigFile(trial->getCondition(i)->ambixConfig);
			
			m_player->play();
			break;
		}
	}

	if (buttonThatWasClicked == &selectReferenceButton)
	{
		if (trial == nullptr)
			return;

		if (timeSyncPlayback)
		{
			m_player->stop();
			trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
			m_player->loadFile(trial->getReference(0)->filepath);
			m_player->setGain(trial->getCondition(0)->gain);
			m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());
			m_player->play();
		}
		else
		{
			m_player->loadFile(trial->getReference(0)->filepath);
			m_player->setGain(trial->getReference(0)->gain);
			m_player->play();
		}

		m_oscTxRx->sendOscMessage("/ts26259/button", (String) "A", (int)1);
		m_oscTxRx->sendOscMessage("/ts26259/button", (String) "B", (int)0);
	}
	else if (buttonThatWasClicked == &prevTrialButton)
	{
		int currentIndex = m_testSession->getCurrentTrialIndex();

		if (currentIndex > 0)
		{
			currentIndex--;
			m_testSession->setCurrentTrialIndex(currentIndex);
			loadTrial(currentIndex);

			if (currentIndex < m_testSession->getNumberOfTrials() - 1)
				nextTrialButton.setEnabled(true); 
			
			if (currentIndex == 0)
				prevTrialButton.setEnabled(false);
		}
	}
	else if (buttonThatWasClicked == &nextTrialButton)
	{
		int currentIndex = m_testSession->getCurrentTrialIndex();

		if (currentIndex < m_testSession->getNumberOfTrials() - 1)
		{
			currentIndex++;
			m_testSession->setCurrentTrialIndex(currentIndex);
			loadTrial(currentIndex);

			if (currentIndex > 0)
				prevTrialButton.setEnabled(true);
		}

		if (currentIndex == m_testSession->getNumberOfTrials() - 1)
		{
			// we can end, once we get to the end
			endTestButton.setEnabled(true);
			nextTrialButton.setEnabled(false);
		}
	}
	else if (buttonThatWasClicked == &endTestButton)
	{
		// save up and close
		m_player->stop();

		m_testSession->exportResults();

		mushraTestListeners.call([this](Listener& l) { l.testCompleted(); });

		setVisible(false);
	}
}

void MushraComponent::sliderValueChanged(Slider* sliderThatWasChanged)
{
	bool rateSampleSliderChanged = sliderThatWasChanged->getProperties()["ratingSlider"];

	if (rateSampleSliderChanged)
	{
		TestTrial* trial = m_testSession->getTrial(m_testSession->getCurrentTrialIndex());

		int sliderIndex = sliderThatWasChanged->getProperties()["sliderIndex"];
		trial->getCondition(sliderIndex)->score = sliderThatWasChanged->getValue();
		ratingReadouts[sliderIndex]->setText(String(sliderThatWasChanged->getValue()), NotificationType::dontSendNotification);
	}
}

void MushraComponent::oscMessageReceived(const OSCMessage& message)
{
	// CONTROL TS26.258 BUTTONS
	if (message.size() == 1 && message.getAddressPattern() == "/ts26259/button" && message[0].isString())
	{
		if (message[0].getString() == "reference")
		{
			selectReferenceButton.triggerClick();
		}
		else if (message[0].getString() == "prev_trial")
		{
			prevTrialButton.triggerClick();
		}
		else if (message[0].getString() == "next_trial")
		{
			nextTrialButton.triggerClick();
		}
	}

	// CONTROL TS26.258 SLIDERS
	if (message.size() == 2 && message.getAddressPattern() == "/ts26259/slider" && message[0].isFloat32() && message[1].isFloat32())
	{
		ratingSliderArray[(int)message[0].getFloat32()]->setValue(message[1].getFloat32());
	}
}

void MushraComponent::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == m_player)
	{
		if (m_player->checkPlaybackStatus())
		{
			m_oscTxRx->sendOscMessage("/ts26259/button", (String) "play", (int)1);
			m_oscTxRx->sendOscMessage("/ts26259/button", (String) "stop", (int)0);
		}
		else
		{
			m_oscTxRx->sendOscMessage("/ts26259/button", (String) "play", (int)0);
			m_oscTxRx->sendOscMessage("/ts26259/button", (String) "stop", (int)1);
		}
	}
}

////==============================================================================
//void MushraComponent::paint (Graphics& g)
//{
//	// BACKGROUND
//	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
//
//	// RECTANGULAR OUTLINE
//	g.setColour(Colours::black);
//	g.drawRect(getLocalBounds(), 1);
//
//    g.setFont (Font (16.0f));
//    g.setColour (Colours::black);
//	g.drawText("MUSHRA Test Mode", 25, 25, 200, 25, Justification::centredLeft, true);
//	g.setFont(Font(14.0f));
//    g.drawText("Trial: " + String(currentTrialIndex + 1) + " of " + String(regionArray.size()), 390, 680, 200, 20, Justification::centredLeft, true);
//	g.drawText("Region: " + String(currentRegion + 1) + " of " + String(regionArray.size()), 390, 700, 200, 20, Justification::centredLeft, true);
//	g.drawText("Sample: " + String(currentTrack + 1) + " of " + String(numberOfSamplesPerRegion), 390, 720, 200, 20, Justification::centredLeft, true);
//    g.drawText("Start: " + String(regionArray[currentRegion]->dawStartTime), 545, 700, 200, 20, Justification::centredLeft, true);
//    g.drawText("Stop: " + String(regionArray[currentRegion]->dawStopTime), 545, 720, 200, 20, Justification::centredLeft, true);
//
//
//	int linesStartX = 125;
//	int linesEndX = 925;
//	int linesStartY = 0.06 * getHeight();
//	int linesEndY = 0.66 * getHeight();
//	int linesYinterval = (linesEndY - linesStartY) / 5;
//
//	Line<float> line1(Point<float>(linesStartX, linesStartY), Point<float>(linesEndX, linesStartY));
//	Line<float> line2(Point<float>(linesStartX, linesStartY + linesYinterval * 1), Point<float>(linesEndX, linesStartY + linesYinterval * 1));
//	Line<float> line3(Point<float>(linesStartX, linesStartY + linesYinterval * 2), Point<float>(linesEndX, linesStartY + linesYinterval * 2));
//	Line<float> line4(Point<float>(linesStartX, linesStartY + linesYinterval * 3), Point<float>(linesEndX, linesStartY + linesYinterval * 3));
//	Line<float> line5(Point<float>(linesStartX, linesStartY + linesYinterval * 4), Point<float>(linesEndX, linesStartY + linesYinterval * 4));
//	Line<float> line6(Point<float>(linesStartX, linesEndY), Point<float>(linesEndX, linesEndY));
//
//	float dashPattern[2];
//	dashPattern[0] = 4.0;
//	dashPattern[1] = 8.0;
//	g.setColour(Colours::dimgrey);
//
//	//g.drawDashedLine(line1, dashPattern, 2, 1.0f);
//	g.drawDashedLine(line2, dashPattern, 2, 1.0f);
//	g.drawDashedLine(line3, dashPattern, 2, 1.0f);
//	g.drawDashedLine(line4, dashPattern, 2, 1.0f);
//	g.drawDashedLine(line5, dashPattern, 2, 1.0f);
//	//g.drawDashedLine(line6, dashPattern, 2, 1.0f);
//
//	int textStartX = linesStartX - 60;
//	int textWidth = 80;
//	g.drawText("Excellent", textStartX, linesStartY, textWidth, linesYinterval, Justification::centredRight, true);
//	g.drawText("Good", textStartX, linesStartY + linesYinterval * 1, textWidth, linesYinterval, Justification::centredRight, true);
//	g.drawText("Fair", textStartX, linesStartY + linesYinterval * 2, textWidth, linesYinterval, Justification::centredRight, true);
//	g.drawText("Poor", textStartX, linesStartY + linesYinterval * 3, textWidth, linesYinterval, Justification::centredRight, true);
//	g.drawText("Bad", textStartX, linesStartY + linesYinterval * 4, textWidth, linesYinterval, Justification::centredRight, true);
//
//}
//
//void MushraComponent::createGui()
//{
//	int numberOfRegions = 4;
//	for (int i = 0; i < numberOfRegions; ++i)
//	{
//		regionArray.add(new SampleRegion());
//		regionArray[i]->dawStartTime = 0.0f; //markerTimeArray[i * 2];         // 0 2 4 6
//		regionArray[i]->dawStopTime = 5.0f;  //markerTimeArray[(i * 2) + 1];    // 1 3 5 7
//		regionArray[i]->calculateStartEndTimes();
//	}
//
//	int numberOfSamplesPerRegion = 8;
//	numberOfSamplesPerRegion = numberOfSamplesPerRegion;
//	
//	
//	// scores array
//	scoresArray.resize(regionArray.size());
//	for (int i = 0; i < regionArray.size(); ++i) scoresArray[i].resize(numberOfSamplesPerRegion);
//
//	// Put some values in
//	//scoresArray[1][2] = 6.0;
//	//scoresArray[3][1] = 5.5;
//
//	// trial randomization array
//	trialRandomizationArray.resize(regionArray.size());
//	for (int i = 0; i < regionArray.size(); ++i) trialRandomizationArray[i] = i;
//	std::random_device seed;
//	std::mt19937 rng(seed());
//	std::shuffle(trialRandomizationArray.begin(), trialRandomizationArray.end(), rng);
//	currentRegion = trialRandomizationArray[currentTrialIndex]; // set the first region according to the randomized order
//	
//	// sample randomization array
//	sampleRandomizationArray.resize(regionArray.size());
//	for (int i = 0; i < regionArray.size(); ++i)
//	{
//		sampleRandomizationArray[i].resize(numberOfSamplesPerRegion);
//		for (int j = 0; j < numberOfSamplesPerRegion; ++j) sampleRandomizationArray[i][j] = j;
//        
//        std::random_device seed;
//        std::mt19937 rng(seed());
//        std::shuffle(sampleRandomizationArray[i].begin(), sampleRandomizationArray[i].end(), rng);
//	}
//	
//	for (int i = 0; i < numberOfSamplesPerRegion; ++i)
//	{
//		rateSampleSliderArray.add(new Slider());
//		rateSampleSliderArray[i]->getProperties().set("rateSampleSlider", true);
//		rateSampleSliderArray[i]->getProperties().set("sliderIndex", i);
//        rateSampleSliderArray[i]->setSliderStyle(Slider::LinearBarVertical);
//        rateSampleSliderArray[i]->setTextBoxIsEditable(false);
//        rateSampleSliderArray[i]->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
//		rateSampleSliderArray[i]->setRange(0, 100, 1);
//		rateSampleSliderArray[i]->setValue(0);
//		rateSampleSliderArray[i]->addListener(this);
//		addAndMakeVisible(rateSampleSliderArray[i]);
//	}
//
//    for (int i = 0; i < numberOfSamplesPerRegion; ++i)
//    {
//        rateSampleSliderLabelArray.add(new Label());
//        rateSampleSliderLabelArray[i]->setText(String(scoresArray[currentRegion][i],0), dontSendNotification);
//        rateSampleSliderLabelArray[i]->setJustificationType(Justification::centred);
//		rateSampleSliderLabelArray[i]->setColour(Label::textColourId, Colours::black);
//        addAndMakeVisible(rateSampleSliderLabelArray[i]);
//    }
//    
//	StringArray playButtonAlphabet = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
//                                    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};
//	for (int i = 0; i < numberOfSamplesPerRegion; ++i)
//	{
//		playSampleButtonArray.add(new TextButton());
//		playSampleButtonArray[i]->getProperties().set("playSampleButton", true);
//		playSampleButtonArray[i]->getProperties().set("buttonIndex", i);
//		if (i < playButtonAlphabet.size()) playSampleButtonArray[i]->setButtonText(playButtonAlphabet[i]);
//		playSampleButtonArray[i]->addListener(this);
//		addAndMakeVisible(playSampleButtonArray[i]);
//	}
//
//	playReference.setButtonText("Reference");
//	playReference.addListener(this);
//	if (isReferenceButtonVisible) addAndMakeVisible(playReference);
//
//	loopSlider.setSliderStyle(Slider::ThreeValueHorizontal);
//	loopSlider.setRange(0, 1);
//	loopSlider.setMinAndMaxValues(0, 1);
//	loopSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
//	loopSlider.addListener(this);
//	addAndMakeVisible(loopSlider);
//
//	loopTB.setButtonText("Loop");
//	loopTB.setToggleState(loop, dontSendNotification);
//	loopTB.addListener(this);
//	loopTB.setColour(ToggleButton::textColourId, Colours::black);
//	loopTB.setColour(ToggleButton::tickColourId, Colours::black);
//	loopTB.setColour(ToggleButton::tickDisabledColourId, Colours::black);
//	addAndMakeVisible(loopTB);
//
//	stopPlaybackB.setButtonText("Stop");
//	stopPlaybackB.addListener(this);
//	addAndMakeVisible(stopPlaybackB);
//
//	goToPrevious.setButtonText("Previous Trial");
//	goToPrevious.addListener(this);
//	addAndMakeVisible(goToPrevious);
//
//	goToNext.setButtonText("Next Trial");
//	goToNext.addListener(this);
//	addAndMakeVisible(goToNext);
//
//
//	dawTimeLabel.setColour(Label::textColourId, Colours::black);
//	sampleTimeLabel.setColour(Label::textColourId, Colours::black);
//	addAndMakeVisible(dawTimeLabel);
//	addAndMakeVisible(sampleTimeLabel);
//
//	updateTransportSlider(true);
//	updateClientTransportSlider();
//	updateClientRatingSliders();
//
//	resized();
//}
//
//void MushraComponent::connectOsc(String dawIp, String clientIp, int dawTxPort, int dawRxPort, int clientTxPort, int clientRxPort)
//{
//	// DAW
//	//dawTx.connectSender(dawIp, dawTxPort);
//	//dawRx.connectReceiver(dawRxPort);
//	//dawRx.addListener(this);
//
//	//// CLIENT
//	//clientTx.connectSender(clientIp, clientTxPort);
//	//clientRx.connectReceiver(clientRxPort);
//	//clientRx.addListener(this);
//
//	clientRxPortAtHost = clientRxPort;
//
//	// DAW state
//	stopPlayback();
//}
//
//void MushraComponent::resized()
//{
//	auto rateRectWidth = 0.8 * getWidth();
//	auto rateRectHeight = 0.6 * getHeight();
//	
//	auto sliderSpacing = rateRectWidth / (numberOfSamplesPerRegion + 1);
//	
//	auto sliderWidth = 50;
//	auto sliderHeight = rateRectHeight;
//	
//	auto buttonWidth = 65;
//	auto buttonHeight = buttonWidth;
//	auto labelHeight = 0.5 * buttonWidth;
//
//
//	auto sliderPositionX = (getWidth() - rateRectWidth) / 2 + sliderWidth / 2;
//	auto sliderPositionY = 0.06 * getHeight() + sliderHeight / 2;
//	auto labelPositionY = sliderPositionY + sliderHeight / 2 + labelHeight * 0.5;
//	auto buttonPositionY = sliderPositionY + sliderHeight / 2 + buttonHeight;
//
//	for (int i = 0; i < numberOfSamplesPerRegion; ++i)
//	{
//		rateSampleSliderArray[i]->setSize(sliderWidth, sliderHeight);
//        rateSampleSliderLabelArray[i]->setSize(buttonWidth, labelHeight);
//		playSampleButtonArray[i]->setSize(buttonWidth, buttonHeight);
//
//		rateSampleSliderArray[i]->setCentrePosition(sliderPositionX + (i+1) * sliderSpacing, sliderPositionY);
//		rateSampleSliderLabelArray[i]->setCentrePosition(sliderPositionX + (i+1) * sliderSpacing, labelPositionY);
//		playSampleButtonArray[i]->setCentrePosition(sliderPositionX + (i+1) * sliderSpacing, buttonPositionY);
//	}
//
//	playReference.setBounds(25, buttonPositionY - buttonHeight / 2, 80, buttonHeight);
//	stopPlaybackB.setBounds(25, buttonPositionY + buttonHeight, 80, buttonHeight);
//	loopSlider.setBounds(sliderPositionX, buttonPositionY + buttonHeight, rateRectWidth, labelHeight);
//
//	loopTB.setBounds(0.8 * getWidth(), 0.875 * getHeight(), 80, labelHeight);
//	goToPrevious.setBounds(sliderPositionX, buttonPositionY + 2 * buttonHeight, 100, labelHeight);
//	goToNext.setBounds(sliderPositionX + 125, buttonPositionY + 2 * buttonHeight, 100, labelHeight);
//
//	dawTimeLabel.setBounds(700, 700, 200, 20);
//	sampleTimeLabel.setBounds(700, 720, 200, 20);
//}
//
//void MushraComponent::buttonClicked (Button* buttonThatWasClicked)
//{
//	bool playSampleButtonState = buttonThatWasClicked->getProperties()["playSampleButton"];
//    if (playSampleButtonState)
//	{
//		int buttonIndex = buttonThatWasClicked->getProperties()["buttonIndex"];
//		playSample(buttonIndex, randomizeSamples);
//	}
//	else if (buttonThatWasClicked == &playReference)
//	{
//		playSample(0, false);
//	}
//	else if (buttonThatWasClicked == &stopPlaybackB)
//	{
//        stopPlayback();
//	}
//	else if (buttonThatWasClicked == &loopTB)
//	{
//		loop = loopTB.getToggleState();
//	}
//	else if (buttonThatWasClicked == &goToNext)
//	{
//		if (currentTrialIndex < regionArray.size() - 1)
//        {
//            currentTrialIndex++;
//			currentRegion = trialRandomizationArray[currentTrialIndex];
//            stopPlayback();
//            updateTransportSlider(true);
//			updateClientTransportSlider();
//			updateRatingSliders();
//			updateClientRatingSliders();
//        }
//
//	}
//	else if (buttonThatWasClicked == &goToPrevious)
//	{
//		if (currentTrialIndex > 0)
//        {
//			currentTrialIndex--;
//			currentRegion = trialRandomizationArray[currentTrialIndex];
//			stopPlayback();
//            updateTransportSlider(true);
//			updateClientTransportSlider();
//            updateRatingSliders();
//			updateClientRatingSliders();
//        }
//	}
//	repaint();
//}
//
//void MushraComponent::sliderValueChanged(Slider *sliderThatWasChanged)
//{
//	bool rateSampleSliderChanged = sliderThatWasChanged->getProperties()["rateSampleSlider"];
//
//	if (rateSampleSliderChanged)
//	{
//		int sliderIndex = sliderThatWasChanged->getProperties()["sliderIndex"];
//        int trackNum = sampleRandomizationArray[currentRegion][sliderIndex];
//		scoresArray[currentRegion][trackNum] = sliderThatWasChanged->getValue();
//		
//        for (int i = 0; i < numberOfSamplesPerRegion; ++i)
//        {
//            int trackNum = sampleRandomizationArray[currentRegion][i];
//            rateSampleSliderLabelArray[i]->setText(String(scoresArray[currentRegion][trackNum],0), dontSendNotification);
//        }
//	}
//	else if (sliderThatWasChanged == &loopSlider)
//	{
//		float regionLength = regionArray[currentRegion]->getRegionLength();
//		regionArray[currentRegion]->loopStartOffset = loopSlider.getMinValue();
//		regionArray[currentRegion]->loopEndOffset = regionLength - loopSlider.getMaxValue();
//		regionArray[currentRegion]->calculateStartEndTimes();
//		updateClientTransportSlider();
//	}
//}
//
//void MushraComponent::playSample(int track, bool randomize)
//{
////    if (randomize) track = sampleRandomizationArray[currentRegion][track];
////
////    if (playback == true)         stopPlayback(); //dawTx.sendOscMessage("/stop");
////    dawTx.sendOscMessage("/time", regionArray[currentRegion]->startTime);
////    if (track != currentTrack)
////    {
////        dawTx.sendOscMessage("/track/" + String(currentTrack + 1) + "/solo", 0);
////    }
////    dawTx.sendOscMessage("/track/" + String(track + 1) + "/solo", 1);
////    dawTx.sendOscMessage("/play");
////    playback = true;
////    currentTrack = track;
//}
//
//void MushraComponent::playSampleLoop()
//{
////    dawTx.sendOscMessage("/track/" + String(0) + "/mute", 1);
////    dawTx.sendOscMessage("/time", regionArray[currentRegion]->startTime);
////    //Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 100 );
////    dawTx.sendOscMessage("/track/" + String(0) + "/mute", 0);
//}
//
//void MushraComponent::stopPlayback()
//{
////    dawTx.sendOscMessage("/track/" + String(0) + "/mute", 1);
////    dawTx.sendOscMessage("/stop");
////    currentPosition = regionArray[currentRegion]->startTime;
////    dawTx.sendOscMessage("/time", currentPosition);
////    dawTx.sendOscMessage("/soloreset");
////    //Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 100);
////    dawTx.sendOscMessage("/track/" + String(0) + "/mute", 0);
//}
//
//void MushraComponent::oscMessageReceived(const OSCMessage& message)
//{
//    // DAW
//	
//	if (message.size() == 1 && message.getAddressPattern() == "/time" && message[0].isFloat32())
//    {
//        currentPosition = message[0].getFloat32();
//        updateTransportSlider(false);
//		updateClientTransportSlider();
//        
//        dawTimeLabel.setText("DAW Time: " + String(currentPosition, 3), NotificationType::dontSendNotification);
//		sampleTimeLabel.setText("Sample Time: " + String(currentPosition - regionArray[currentRegion]->startTime, 3), NotificationType::dontSendNotification);
//		
//        if(currentPosition >= regionArray[currentRegion]->stopTime)
//        {
//            if(loop) playSampleLoop();
//            else stopPlayback();
//        }
//    }
//
//
//	// CLIENT
//	if (message.size() == 1 && message.getAddressPattern() == "/host/play" && message[0].isInt32())
//	{
//		playSampleButtonArray[message[0].getInt32()]->triggerClick();
//	}
//	else if (message.size() == 1 && message.getAddressPattern() == "/host" && message[0].isString())
//	{
//		if (message[0].getString() == "playref") playReference.triggerClick();
//		if (message[0].getString() == "stopplayback") stopPlaybackB.triggerClick();
//		if (message[0].getString() == "loopon") loopTB.setToggleState(true, sendNotification);
//		if (message[0].getString() == "loopoff") loopTB.setToggleState(false, sendNotification);
//		if (message[0].getString() == "gonext") goToNext.triggerClick();
//		if (message[0].getString() == "goprev") goToPrevious.triggerClick();
//	}
//	else if (message.size() == 2 && message.getAddressPattern() == "/host/slider" && message[0].isInt32() && message[1].isFloat32())
//	{
//		rateSampleSliderArray[message[0].getInt32()]->setValue(message[1].getFloat32());
//	}
//    else if (message.size() == 4 && message.getAddressPattern() == "/host/loopslider" && message[0].isFloat32() && message[1].isFloat32() && message[2].isFloat32() && message[3].isFloat32())
//    {
//        regionArray[currentRegion]->loopStartOffset = message[2].getFloat32();
//        regionArray[currentRegion]->loopEndOffset = message[0].getFloat32() - message[3].getFloat32();
//		regionArray[currentRegion]->calculateStartEndTimes();
//        updateTransportSlider(true);
//    }
//}
//
//void MushraComponent::updateTransportSlider(bool updateLoop)
//{
//	float regionLength = regionArray[currentRegion]->getRegionLength();
//	if (updateLoop)
//	{
//        loopSlider.setRange(0, regionLength, dontSendNotification);
//        loopSlider.setValue(regionArray[currentRegion]->loopStartOffset, dontSendNotification);
//		loopSlider.setMinValue(regionArray[currentRegion]->loopStartOffset, dontSendNotification);
//		loopSlider.setMaxValue(regionLength - regionArray[currentRegion]->loopEndOffset, dontSendNotification);
//	}
//	else
//	{
//		loopSlider.setValue(currentPosition - regionArray[currentRegion]->dawStartTime, dontSendNotification);
//	}
//}
//
//void MushraComponent::initClientGui()
//{
//	clientTx.sendOscMessage(String("/client/hostip"), hostIp);
//	clientTx.sendOscMessage(String("/client/hostport"), clientRxPortAtHost);
//	clientTx.sendOscMessage(String("/client/nofsamples"), numberOfSamplesPerRegion);
//	clientTx.sendOscMessage(String("/client/currenttrial/"), (int)currentTrialIndex, (int)regionArray.size());
//	clientTx.sendOscMessage(String("/client/isrefbutton"), 1);
//	clientTx.sendOscMessage(String("/client/gui"), String("create"));
//}
//
//void MushraComponent::updateClientTransportSlider()
//{
////    float regionLength = regionArray[currentRegion]->getRegionLength();
////    clientTx.sendOscMessage("/client/loopslider/", regionLength, currentPosition - regionArray[currentRegion]->dawStartTime, regionArray[currentRegion]->loopStartOffset, regionLength - regionArray[currentRegion]->loopEndOffset);
//}
//
//void MushraComponent::updateClientRatingSliders()
//{
////    for (int sliderIndex = 0; sliderIndex < rateSampleSliderArray.size(); ++sliderIndex)
////    {
////        clientTx.sendOscMessage("/client/slider/", (int)sliderIndex, (float)rateSampleSliderArray[sliderIndex]->getValue());
////    }
////    clientTx.sendOscMessage("/client/currenttrial/", (int)currentTrialIndex, (int)regionArray.size());
//}
//
//void MushraComponent::updateRatingSliders()
//{
//	for (int i = 0; i < numberOfSamplesPerRegion; ++i)
//	{
//        int trackNum = sampleRandomizationArray[currentRegion][i];
//		rateSampleSliderArray[i]->setValue(scoresArray[currentRegion][trackNum]);
//	}
//}
//
//void MushraComponent::saveResults()
//{
//	File chosenFile;
//	String dataToSave;
//
//#if JUCE_MODAL_LOOPS_PERMITTED
//	FileChooser fc("Choose a file to save...", File::getCurrentWorkingDirectory(),
//		"*.txt", true);
//	if (fc.browseForFileToSave(true))
//	{
//		chosenFile = fc.getResult().withFileExtension(".txt");
//
//		dataToSave = String("##### RAW RESULTS #####\n\n");
//		for (int i = 0; i < scoresArray.size(); i++)
//		{
//			for (int j = 0; j < scoresArray[0].size(); j++)
//			{
//				dataToSave += String(scoresArray[i][j]);
//				dataToSave += "\t";
//			}
//			dataToSave += "\n";
//		}
//
//		dataToSave += String("\n");
//		dataToSave += String("##### TRIAL RANDOMIZATION ARRAY #####\n\n");
//		for (int i = 0; i < scoresArray.size(); i++)
//		{
//			dataToSave += String(trialRandomizationArray[i]);
//			dataToSave += "\t";
//		}
//		dataToSave += String("\n\n");
//		dataToSave += String("##### SAMPLE RANDOMIZATION ARRAY #####\n\n");
//		for (int i = 0; i < scoresArray.size(); i++)
//		{
//			for (int j = 0; j < scoresArray[0].size(); j++)
//			{
//				dataToSave += String(sampleRandomizationArray[i][j]);
//				dataToSave += "\t";
//			}
//			dataToSave += "\n";
//		}
//
//		dataToSave += "\n" + String("##### TRACK NAMES #####\n\n");
//		for (int i = 0; i < trackNameArray.size(); i++)
//		{
//			dataToSave += trackNameArray[i] + "\n";
//		}
//		dataToSave += "\n" + String("##### DATE AND TIME #####\n\n");
//		dataToSave += Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S") + "\n";
//
//		dataToSave += "\n" + String("##### REGIONS #####\n\n");
//		for (int i = 0; i < regionArray.size(); i++)
//		{
//			dataToSave += "Region: " + String(i + 1) + "\n";
//			dataToSave += "Beginning: " + String(regionArray[i]->dawStartTime) + "\n";
//			dataToSave += "End: " + String(regionArray[i]->dawStopTime) + "\n\n";
//		}
//		chosenFile.replaceWithText(dataToSave);
//	}
//#endif
//}
