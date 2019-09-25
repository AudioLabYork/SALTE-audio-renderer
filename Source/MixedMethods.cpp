#include "MixedMethods.h"

MixedMethodsComponent::MixedMethodsComponent()
	: m_oscTxRx(nullptr)
	, m_player(nullptr)
	, m_renderer(nullptr)
	, m_testSession(nullptr)
	, leftBorder(15)
	, rightBorder(15)
	, topBorder(15)
	, bottomBorder(15)
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
	selectReferenceButton.setColour(TextButton::buttonOnColourId, Colours::red);
	selectReferenceButton.addListener(this);
	addChildComponent(selectReferenceButton);

	selectTConditionAButton.setButtonText("A");
	selectTConditionAButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
	selectTConditionAButton.setColour(TextButton::buttonOnColourId, Colours::red);
	selectTConditionAButton.addListener(this);
	addChildComponent(selectTConditionAButton);

	selectTConditionBButton.setButtonText("B");
	selectTConditionBButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
	selectTConditionBButton.setColour(TextButton::buttonOnColourId, Colours::red);
	selectTConditionBButton.addListener(this);
	addChildComponent(selectTConditionBButton);
}

MixedMethodsComponent::~MixedMethodsComponent()
{
}

void MixedMethodsComponent::init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRenderer* renderer)
{
	m_oscTxRx = oscTxRx;
	m_oscTxRx->addListener(this);
	
	m_renderer = renderer;
	
	m_player = player;
	m_player->addChangeListener(this);
}

void MixedMethodsComponent::loadTestSession(TestSession* testSession)
{
	m_testSession = testSession;
	loadTrial(0);
}

void MixedMethodsComponent::addListener(Listener* newListener)
{
	mushraTestListeners.add(newListener);
}

void MixedMethodsComponent::removeListener(Listener* listener)
{
	mushraTestListeners.remove(listener);
}

void MixedMethodsComponent::sendMsgToLogWindow(String message)
{
	currentMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
}

void MixedMethodsComponent::loadTrial(int trialIndex)
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
			ratingSliderArray[i]->getProperties().set("ratingType", "condition");
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
			selectConditionButtonArray[i]->setColour(TextButton::buttonOnColourId, Colours::red);
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
			ratingSliderArray[i]->getProperties().set("ratingType", "attribute");
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

	// ######################## DISTRIBUTE ELEMENTS IN THE GUI ########################
	const int topMargin = 50;
	const int bottomMargin = 125;
	const int leftMargin = 120;
	testArea.setBounds(testSpace.getX() + leftMargin, testSpace.getY() + topMargin, testSpace.getWidth() - leftMargin, testSpace.getHeight() - (topMargin + bottomMargin));

	// position sliders
	if (trial->getNumberOfMConditions() > 0 || trial->getNumberOfTAttributes() > 0)
	{
		const int numberOfSliders = trial->getNumberOfMConditions() + trial->getNumberOfTAttributes();
		const int sliderWidth = 30;
		const int sliderHeight = testArea.getHeight();
		const int inc = testArea.getWidth() / (numberOfSliders + 1);

		for (int i = 0; i < numberOfSliders; ++i)
		{
			int sliderPositionX = testArea.getX() + (i + 1) * inc - sliderWidth / 2;
			ratingSliderArray[i]->setBounds(sliderPositionX, testArea.getY(), sliderWidth, sliderHeight);
			if (ratingReadouts[i] != nullptr) ratingReadouts[i]->setBounds(sliderPositionX - 8, testArea.getBottom() + 5, sliderWidth + 16, 20);
			if (selectConditionButtonArray[i] != nullptr) selectConditionButtonArray[i]->setBounds(sliderPositionX, testArea.getBottom() + 30, sliderWidth, 25);
			if (attributeRatingLabels[i] != nullptr) attributeRatingLabels[i]->setBounds(sliderPositionX - 50 + sliderWidth / 2, testArea.getBottom() + 30, 100, 25);

		}
	}


	// position MUSHRA reference / TS26259 conditions button
	int referencePositionX = ratingSliderArray.getFirst()->getX();
	int referenceWidth = ratingSliderArray.getLast()->getX() + ratingSliderArray.getLast()->getWidth() - referencePositionX;
	selectReferenceButton.setBounds(referencePositionX, testArea.getBottom() + 65, referenceWidth, 25);
	selectTConditionAButton.setBounds(referencePositionX, testArea.getBottom() + 65, referenceWidth / 3, 25);
	selectTConditionBButton.setBounds(referencePositionX + 2 * referenceWidth / 3, testArea.getBottom() + 65, referenceWidth / 3, 25);

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
	updateRemoteInterface();
}

void MixedMethodsComponent::paint(Graphics& g)
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
		g.drawFittedText(trial->getTrialName(), testArea.getX(), testSpace.getY(), testArea.getWidth(), 20, Justification::centred, 1);
		g.drawFittedText(trial->getTrialInstruction(), testArea.getX(), testSpace.getY() + 20, testArea.getWidth(), 20, Justification::centred, 1);

		// paint rating scale labels and horizontal lines
		StringArray ratings = trial->getRatingOptions();
		if (!ratings.isEmpty())
		{
			g.setFont(14.0f);
			
			// check the maximum width of rating labels
			int maxTextWidth = 0;
			for (int i = 0; i < ratings.size(); ++i)
			{
				int size = g.getCurrentFont().getStringWidth(ratings[i]);
				if (size > maxTextWidth)
					maxTextWidth = size;
			}

			const int textStartX = leftBorder;
			const int textStartY = testArea.getY();
			int ratingLabelsTextWidth = testArea.getX() - leftBorder;
			if (maxTextWidth > ratingLabelsTextWidth) ratingLabelsTextWidth = maxTextWidth;
			int ySpacer = testArea.getHeight() / ratings.size();
			for (int i = 0; i < ratings.size(); ++i)
				g.drawFittedText(ratings[i], textStartX, textStartY + ySpacer * i, ratingLabelsTextWidth, ySpacer, Justification::centredRight, 1);

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

void MixedMethodsComponent::resized()
{
	testSpace.setBounds(leftBorder, topBorder, getWidth() - (leftBorder + rightBorder), getHeight() - (topBorder + bottomBorder));
	prevTrialButton.setBounds(testSpace.getX(), testSpace.getBottom() - 25, 120, 25);
	nextTrialButton.setBounds(testSpace.getX() + 120 + 10, testSpace.getBottom() - 25, 120, 25);
	endTestButton.setBounds(testSpace.getX() + testSpace.getWidth() - 120, testSpace.getBottom() - 25, 120, 25);
}

void MixedMethodsComponent::buttonClicked(Button* buttonThatWasClicked)
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

			// light the button
			selectConditionButtonArray[i]->setToggleState(true, dontSendNotification);
		}
		else selectConditionButtonArray[i]->setToggleState(false, dontSendNotification);
	}

	if (buttonThatWasClicked == &selectReferenceButton)
	{
		if (trial == nullptr)
			return;

		m_player->pause();
		sendMsgToLogWindow(trial->getMReference(0)->name);
		if (timeSyncPlayback) trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
		m_player->loadFile(trial->getMReference(0)->filepath);
		m_player->setGain(trial->getMReference(0)->gain);
		if (timeSyncPlayback) m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());
		m_renderer->setOrder(trial->getMReference(0)->renderingOrder);
		m_renderer->loadFromAmbixConfigFile(trial->getMReference(0)->ambixConfig);
		m_player->play();

		selectReferenceButton.setToggleState(true, dontSendNotification);
	}
	else selectReferenceButton.setToggleState(false, dontSendNotification);

	if (buttonThatWasClicked == &selectTConditionAButton)
	{
		if (trial == nullptr)
			return;

		m_player->pause();
		sendMsgToLogWindow(trial->getTCondition(0)->name);
		if (timeSyncPlayback) trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
		m_player->loadFile(trial->getTCondition(0)->filepath);
		m_player->setGain(trial->getTCondition(0)->gain);
		if (timeSyncPlayback) m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());
		m_renderer->setOrder(trial->getTCondition(0)->renderingOrder);
		m_renderer->loadFromAmbixConfigFile(trial->getTCondition(0)->ambixConfig);
		m_player->play();

		selectTConditionAButton.setToggleState(true, dontSendNotification);
	}
	else selectTConditionAButton.setToggleState(false, dontSendNotification);

	if (buttonThatWasClicked == &selectTConditionBButton)
	{
		if (trial == nullptr)
			return;

		m_player->pause();
		sendMsgToLogWindow(trial->getTCondition(1)->name);
		if (timeSyncPlayback) trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
		m_player->loadFile(trial->getTCondition(1)->filepath);
		m_player->setGain(trial->getTCondition(1)->gain);
		if (timeSyncPlayback) m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());
		m_renderer->setOrder(trial->getTCondition(1)->renderingOrder);
		m_renderer->loadFromAmbixConfigFile(trial->getTCondition(1)->ambixConfig);
		m_player->play();

		selectTConditionBButton.setToggleState(true, dontSendNotification);
	}
	else selectTConditionBButton.setToggleState(false, dontSendNotification);

	if (buttonThatWasClicked == &prevTrialButton)
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

//void MixedMethodsComponent::triggerConditionPlayback(int buttonIndex)
//{
//	this probably requires some macros to call the respective methods:
//	trial->getMCondition(buttonIndex), trial->getTAttribute(buttonIndex), etc.
//}

void MixedMethodsComponent::sliderValueChanged(Slider* sliderThatWasChanged)
{
	bool rateSampleSliderChanged = sliderThatWasChanged->getProperties()["ratingSlider"];

	if (rateSampleSliderChanged)
	{
		TestTrial* trial = m_testSession->getTrial(m_testSession->getCurrentTrialIndex());

		int sliderIndex = sliderThatWasChanged->getProperties()["sliderIndex"];
		String ratingType = sliderThatWasChanged->getProperties()["ratingType"];
		if (ratingType == "condition") trial->getMCondition(sliderIndex)->score = sliderThatWasChanged->getValue();
		if (ratingType == "attribute") trial->getTAttribute(sliderIndex)->score = sliderThatWasChanged->getValue();
		ratingReadouts[sliderIndex]->setText(String(sliderThatWasChanged->getValue()), NotificationType::dontSendNotification);
	}
}

void MixedMethodsComponent::sliderDragStarted(Slider* sliderThatHasBeenStartedDragging)
{
	bool rateSampleSliderChanged = sliderThatHasBeenStartedDragging->getProperties()["ratingSlider"];
	String ratingType = sliderThatHasBeenStartedDragging->getProperties()["ratingType"];

	if (rateSampleSliderChanged && ratingType == "condition")
	{
		int sliderIndex = sliderThatHasBeenStartedDragging->getProperties()["sliderIndex"];
		if (selectConditionButtonArray[sliderIndex] != nullptr) selectConditionButtonArray[sliderIndex]->triggerClick();
	}
}

void MixedMethodsComponent::updateRemoteInterface()
{
	// Transport controls
	if (m_player->checkPlaybackStatus())
	{
		m_oscTxRx->sendOscMessage("/button", (String) "play", (int)1);
		m_oscTxRx->sendOscMessage("/button", (String) "stop", (int)0);
	}
	else
	{
		m_oscTxRx->sendOscMessage("/button", (String) "play", (int)0);
		m_oscTxRx->sendOscMessage("/button", (String) "stop", (int)1);
	}

	if (m_player->checkLoopStatus())
	{
		m_oscTxRx->sendOscMessage("/button", (String) "loop", (int)1);
	}
	else
	{
		m_oscTxRx->sendOscMessage("/button", (String) "loop", (int)0);
	}

	TestTrial* trial = m_testSession->getTrial(m_testSession->getCurrentTrialIndex());

	if (trial != nullptr)
	{
		// send string to display on the screen
		String screenMessage1 = "Trial " + String(m_testSession->getCurrentTrialIndex() + 1) + " of " + String(m_testSession->getNumberOfTrials());
		m_oscTxRx->sendOscMessage("/screen", (String)screenMessage1, (String)trial->getTrialName() + "\n\n" + trial->getTrialInstruction());
	}


	if (selectTConditionAButton.isVisible() && selectTConditionBButton.isVisible() && ratingSliderArray.size() == 4)
	{
		// update remote sliders
		for (int i = 0; i < ratingSliderArray.size(); ++i)
		{
			if (i < 4) m_oscTxRx->sendOscMessage("/slider", (int)i, (float)ratingSliderArray[i]->getValue());
		}

		// update A / B buttons
		m_oscTxRx->sendOscMessage("/button", (String) "A", (int) selectTConditionAButton.getToggleState());
		m_oscTxRx->sendOscMessage("/button", (String) "B", (int) selectTConditionBButton.getToggleState());

		m_oscTxRx->sendOscMessage("/showUI", (int) 1);
	}
	else
	{
		m_oscTxRx->sendOscMessage("/showUI", (int)0);
	}
	
}

void MixedMethodsComponent::oscMessageReceived(const OSCMessage& message)
{
	// CONTROL BUTTONS
	if (message.size() == 1 && message.getAddressPattern() == "/button" && message[0].isString())
	{
		if (message[0].getString() == "play")
		{
			m_player->play();
		}
		else if (message[0].getString() == "stop")
		{
			m_player->stop();
		}
		else if (message[0].getString() == "loop")
		{
			m_player->loop(!m_player->checkLoopStatus());
		}
		else if (message[0].getString() == "A")
		{
			selectTConditionAButton.triggerClick();
		}
		else if (message[0].getString() == "B")
		{
			selectTConditionBButton.triggerClick();
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

	// CONTROL SLIDERS
	if (message.size() == 2 && message.getAddressPattern() == "/slider" && message[0].isFloat32() && message[1].isFloat32())
	{
		ratingSliderArray[(int)message[0].getFloat32()]->setValue(message[1].getFloat32());
	}
}

void MixedMethodsComponent::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == m_player)
	{
		if(m_testSession != nullptr) updateRemoteInterface();
	}
}
