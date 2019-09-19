#include "TC_MUSHRA.h"

MushraComponent::MushraComponent()
	: m_oscTxRx(nullptr)
	, m_player(nullptr)
	, m_renderer(nullptr)
	, m_testSession(nullptr)
	, leftBorder(15)
	, rightBorder(15)
	, topBorder(15)
	, bottomBorder(15)
	, ratingLabelsTextWidth(130)
{
	prevTrialButton.setButtonText("Previous Trial");
	prevTrialButton.addListener(this);
	addAndMakeVisible(prevTrialButton);

	nextTrialButton.setButtonText("Next Trial");
	nextTrialButton.addListener(this);
	addAndMakeVisible(nextTrialButton);

	endTestButton.setButtonText("End Test");
	endTestButton.addListener(this);
	addAndMakeVisible(endTestButton);

	selectReferenceButton.setButtonText("Reference");
	selectReferenceButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
	selectReferenceButton.setColour(TextButton::buttonOnColourId, Colours::green);
	selectReferenceButton.addListener(this);
	addChildComponent(selectReferenceButton);

	selectTConditionAButton.setButtonText("A");
	selectTConditionAButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
	selectTConditionAButton.setColour(TextButton::buttonOnColourId, Colours::green);
	selectTConditionAButton.addListener(this);
	addChildComponent(selectTConditionAButton);

	selectTConditionBButton.setButtonText("B");
	selectTConditionBButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
	selectTConditionBButton.setColour(TextButton::buttonOnColourId, Colours::green);
	selectTConditionBButton.addListener(this);
	addChildComponent(selectTConditionBButton);
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

void MushraComponent::sendMsgToLogWindow(String message)
{
	currentMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
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

	ratingSliderArray.clear();
	ratingReadouts.clear();
	selectConditionButtonArray.clear();
	attributeRatingLabels.clear();

	// MUSHRA Reference
	selectReferenceButton.setVisible(trial->isMReferencePresent());

	// MUSHRA Conditions
	if (trial->getNumberOfMConditions() > 0)
	{
		StringArray selectButtonAlphabet = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
												"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };

		for (int i = 0; i < trial->getNumberOfMConditions(); ++i)
		{
			ratingSliderArray.add(new Slider());
			ratingSliderArray[i]->getProperties().set("ratingSlider", true);
			ratingSliderArray[i]->getProperties().set("sliderIndex", i);
			ratingSliderArray[i]->setSliderStyle(Slider::LinearBarVertical);
			ratingSliderArray[i]->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
			ratingSliderArray[i]->setTextBoxIsEditable(false);
			ratingSliderArray[i]->setRange(trial->getMCondition(i)->minScore, trial->getMCondition(i)->maxScore, 1);
			ratingSliderArray[i]->setValue(trial->getMCondition(i)->score, dontSendNotification);
			ratingSliderArray[i]->addListener(this);
			addAndMakeVisible(ratingSliderArray[i]);

			ratingReadouts.add(new Label());
			ratingReadouts[i]->setText(String(trial->getMCondition(i)->score), dontSendNotification);
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
	}

	// TS26259 Attributes
	if (trial->getNumberOfTAttributes() > 0)
	{
		for (int i = 0; i < trial->getNumberOfTAttributes(); ++i)
		{
			ratingSliderArray.add(new Slider());
			ratingSliderArray[i]->getProperties().set("ratingSlider", true);
			ratingSliderArray[i]->getProperties().set("sliderIndex", i);
			ratingSliderArray[i]->setSliderStyle(Slider::LinearBarVertical);
			ratingSliderArray[i]->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
			ratingSliderArray[i]->setTextBoxIsEditable(false);
			ratingSliderArray[i]->setRange(trial->getTAttribute(i)->minScore, trial->getTAttribute(i)->maxScore, 0.05);
			ratingSliderArray[i]->setValue(trial->getTAttribute(i)->score, dontSendNotification);
			ratingSliderArray[i]->addListener(this);
			addAndMakeVisible(ratingSliderArray[i]);

			ratingReadouts.add(new Label());
			ratingReadouts[i]->setText(String(trial->getTAttribute(i)->score), dontSendNotification);
			ratingReadouts[i]->setJustificationType(Justification::centred);
			addAndMakeVisible(ratingReadouts[i]);

			attributeRatingLabels.add(new Label());
			attributeRatingLabels[i]->setText(trial->getTAttribute(i)->name, dontSendNotification);
			attributeRatingLabels[i]->setJustificationType(Justification::centred);
			addAndMakeVisible(attributeRatingLabels[i]);
		}
	}

	// TS26259 Conditions
	selectTConditionAButton.setVisible(trial->areTConditionsPresent());
	selectTConditionBButton.setVisible(trial->areTConditionsPresent());

	// DISTRIBUTE ELEMENTS IN THE GUI
	const int topMargin = 50;
	const int bottomMargin = 120;
	testArea.setBounds(testSpace.getX() + ratingLabelsTextWidth, testSpace.getY() + topMargin, testSpace.getWidth() - ratingLabelsTextWidth, testSpace.getHeight() - (topMargin + bottomMargin));

	// position MUSHRA condition sliders
	if (trial->getNumberOfMConditions() > 0)
	{
		const int sliderSpacing = 0;
		const int sliderWidth = 30;
		const int sliderHeight = testArea.getHeight();
		int inc = testArea.getWidth() / trial->getNumberOfMConditions();
		inc = (int) inc + ((inc - sliderWidth) / trial->getNumberOfMConditions());
		const int sliderPositionX = testArea.getX();
		const int sliderPositionY = testArea.getY();

		for (int i = 0; i < trial->getNumberOfMConditions(); ++i)
		{
			ratingSliderArray[i]->setBounds(sliderPositionX + (i * inc), sliderPositionY, sliderWidth, sliderHeight);
			ratingReadouts[i]->setBounds(sliderPositionX + (i * inc), testArea.getBottom() + 5, sliderWidth, 20);
			selectConditionButtonArray[i]->setBounds(sliderPositionX + (i * inc), testArea.getBottom() + 30, sliderWidth, 25);
		}
	}

	// position MUSHRA reference button
	selectReferenceButton.setBounds(testArea.getX(), testArea.getBottom() + 65, testArea.getWidth(), 25);

	// position TS26259 attributes
	if (trial->getNumberOfTAttributes() > 0)
	{
		const int sliderSpacing = 0;
		const int sliderWidth = 30;
		const int sliderHeight = testArea.getHeight();
		int inc = testArea.getWidth() / trial->getNumberOfTAttributes();
		inc = (int)inc + ((inc - sliderWidth) / trial->getNumberOfTAttributes());
		const int sliderPositionX = testArea.getX();
		const int sliderPositionY = testArea.getY();

		for (int i = 0; i < trial->getNumberOfTAttributes(); ++i)
		{
			ratingSliderArray[i]->setBounds(sliderPositionX + (i * inc), sliderPositionY, sliderWidth, sliderHeight);
			ratingReadouts[i]->setBounds(sliderPositionX + (i * inc), testArea.getBottom() + 5, sliderWidth, 20);
			attributeRatingLabels[i]->setBounds(sliderPositionX + (i * inc), testArea.getBottom() + 30, 100, 25);
		}
	}

	// position TS26259 conditions buttons
	selectTConditionAButton.setBounds(testArea.getX(), testArea.getBottom() + 65, testArea.getWidth()/3, 25);
	selectTConditionBButton.setBounds(testArea.getX() + 2 * testArea.getWidth() / 3, testArea.getBottom() + 65, testArea.getWidth()/3, 25);


	// set the looping state of the player
	m_player->loop(trial->getLoopingState());

	// update the session navigation buttons' states
	if (m_testSession->getCurrentTrialIndex() == 0)
		prevTrialButton.setEnabled(false);
	else
		prevTrialButton.setEnabled(true);

	if(m_testSession->getNumberOfTrials() > 1 && m_testSession->getCurrentTrialIndex() < m_testSession->getNumberOfTrials() - 1)
		nextTrialButton.setEnabled(true);
	else
		nextTrialButton.setEnabled(false);
	
	if(m_testSession->getCurrentTrialIndex() == m_testSession->getNumberOfTrials() - 1)
		endTestButton.setEnabled(true);
	else
		endTestButton.setEnabled(false);

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

		g.drawText("Trial " + String(m_testSession->getCurrentTrialIndex() + 1) + " of " + String(m_testSession->getNumberOfTrials()), leftBorder, topBorder, getWidth() - (leftBorder + rightBorder), 20, Justification::centredLeft, true);
		g.setFont(16.0f);
		g.drawText(trial->getTrialName(), testArea.getX(), testSpace.getY(), testArea.getWidth(), 20, Justification::centred, true);
		g.drawText(trial->getTrialInstruction(), testArea.getX(), testSpace.getY() + 20, testArea.getWidth(), 20, Justification::centred, true);

		// paint rating scale labels and horizontal lines
		StringArray ratings = trial->getRatingOptions();
		if (!ratings.isEmpty())
		{
			g.setFont(14.0f);

			//ratingLabelsTextWidth = 100;
			//for (int i = 0; i < ratings.size(); ++i)
			//{
			//	int size = g.getCurrentFont().getStringWidth(ratings[i]);
			//	if (size > ratingLabelsTextWidth)
			//		ratingLabelsTextWidth = size;
			//}

			int ySpacer = testArea.getHeight() / ratings.size();

			const int textStartX = leftBorder;
			const int textStartY = testArea.getY();
			for (int i = 0; i < ratings.size(); ++i)
				g.drawFittedText(ratings[i], textStartX, textStartY + ySpacer * i, ratingLabelsTextWidth, ySpacer, Justification::centred, 1);

			const int linesStartX = testArea.getX();
			const int linesStartY = testArea.getY();
			const int linesWidth = testArea.getWidth();
			const int numLines = ratings.size() + 1;
			const float dashPattern[2] = { 4.0, 8.0 };
			g.setColour(Colours::ghostwhite);
			for (int i = 0; i < numLines; ++i)
				g.drawDashedLine(Line<float>(Point<float>(linesStartX, linesStartY + ySpacer * i), Point<float>(linesStartX + linesWidth, linesStartY + ySpacer * i)), dashPattern, 2, 0.5f);
		}
	}
}

void MushraComponent::resized()
{
	testSpace.setBounds(leftBorder, topBorder, getWidth() - (leftBorder + rightBorder), getHeight() - (topBorder + bottomBorder));
	prevTrialButton.setBounds(testSpace.getX(), testSpace.getBottom() - 25, 80, 25);
	nextTrialButton.setBounds(testSpace.getX() + 80 + 10, testSpace.getBottom() - 25, 80, 25);
	endTestButton.setBounds(testSpace.getX() + 160 + 20, testSpace.getBottom() - 25, 80, 25);
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

			// display the condition name in console output
			sendMsgToLogWindow(trial->getMCondition(i)->name);

			// store the playback head position
			if (timeSyncPlayback) trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
			
			// setup the player
			m_player->loadFile(trial->getMCondition(i)->filepath);
			m_player->setGain(trial->getMCondition(i)->gain);
			if (timeSyncPlayback) m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());

			// setup the renderer
			m_renderer->setOrder(trial->getMCondition(i)->renderingOrder);
			m_renderer->loadFromAmbixConfigFile(trial->getMCondition(i)->ambixConfig);
			
			// play the scene
			m_player->play();
			break;
		}
	}

	if (buttonThatWasClicked == &selectReferenceButton)
	{
		if (trial == nullptr)
			return;

		m_player->pause();
		if (timeSyncPlayback) trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
		m_player->loadFile(trial->getMReference(0)->filepath);
		m_player->setGain(trial->getMReference(0)->gain);
		if (timeSyncPlayback) m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());
		m_player->play();

		//m_oscTxRx->sendOscMessage("/ts26259/button", (String) "A", (int)1);
		//m_oscTxRx->sendOscMessage("/ts26259/button", (String) "B", (int)0);
	}
	else if (buttonThatWasClicked == &prevTrialButton)
	{
		int currentIndex = m_testSession->getCurrentTrialIndex();

		if (currentIndex > 0)
		{
			currentIndex--;
			m_testSession->setCurrentTrialIndex(currentIndex);
			loadTrial(currentIndex);
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
		if (trial->getMCondition(sliderIndex) != nullptr) trial->getMCondition(sliderIndex)->score = sliderThatWasChanged->getValue();
		if (trial->getTAttribute(sliderIndex) != nullptr) trial->getTAttribute(sliderIndex)->score = sliderThatWasChanged->getValue();
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
