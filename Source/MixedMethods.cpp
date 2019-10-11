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

void MixedMethodsComponent::init(StimulusPlayer* player, BinauralRenderer* renderer)
{	
	m_renderer = renderer;
	
	m_player = player;
	m_player->addChangeListener(this);
}

void MixedMethodsComponent::loadTestSession(TestSession* testSession, OscTransceiver* oscTxRx)
{
	m_testSession = testSession;
	m_oscTxRx = oscTxRx;
	m_oscTxRx->addListener(this);
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

	// prepare the player
	m_player->unloadFileFromTransport();
	m_player->cachingLock = true;
	m_player->clearAudioFileCache();
	
	for (int i = 0; i < trial->getNumberOfMConditions(); ++i)
		m_player->cacheAudioFile(trial->getMCondition(i)->filepath);
	
	if (trial->isMReferencePresent())
		m_player->cacheAudioFile(trial->getMReference(0)->filepath);
	
	if (trial->areTConditionsPresent())
	{
		m_player->cacheAudioFile(trial->getTCondition(0)->filepath);
		m_player->cacheAudioFile(trial->getTCondition(1)->filepath);
	}

	m_player->cachingLock = false;

	ratingSliderArray.clear();
	ratingReadouts.clear();
	selectConditionButtonArray.clear();
	attributeRatingLabels.clear();
	selectReferenceButton.setVisible(false);
	selectTConditionAButton.setVisible(false);
	selectTConditionBButton.setVisible(false);

	// MUSHRA Reference
	if (trial->isMReferencePresent())
	{
		selectReferenceButton.setVisible(true);
		selectReferenceButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
		selectReferenceButton.setColour(TextButton::buttonOnColourId, Colours::red);
	}

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
	if (trial->areTConditionsPresent())
	{
		selectTConditionAButton.setVisible(true);
		selectTConditionAButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
		selectTConditionAButton.setColour(TextButton::buttonOnColourId, Colours::red);

		selectTConditionBButton.setVisible(true);
		selectTConditionBButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
		selectTConditionBButton.setColour(TextButton::buttonOnColourId, Colours::red);
	}

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

	int selectTCondButtonWidth = testArea.getWidth() / 5;
	selectTConditionAButton.setBounds(testArea.getX() + selectTCondButtonWidth, testArea.getBottom() + 65, selectTCondButtonWidth, 25);
	selectTConditionBButton.setBounds(testArea.getX() + 3 * selectTCondButtonWidth, testArea.getBottom() + 65, selectTCondButtonWidth, 25);

	// set the looping state of the player
	m_player->loop(trial->getLoopingState());
	m_player->setPlaybackOffsets(0, 0);

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
			sendMsgToLogWindow("Condition name: " + trial->getMCondition(i)->name);

			// store the playback head position
			if (timeSyncPlayback) trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
			
			// setup the player
			m_player->loadFile(trial->getMCondition(i)->filepath);
			m_player->setGain(trial->getMCondition(i)->gain);
			if (timeSyncPlayback) m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());

			// setup the renderer
			m_renderer->setOrder(trial->getMCondition(i)->renderingOrder);
			
			if (BinauralRenderer::initialiseFromAmbix(File(trial->getMCondition(i)->ambixConfig), m_renderer))
				sendMsgToLogWindow("successfully loaded: " + File(trial->getMCondition(i)->ambixConfig).getFileName());
			else
				sendMsgToLogWindow("failed to load AmbiX file");

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
		sendMsgToLogWindow("Condition name: " + trial->getMReference(0)->name);
		
		if (timeSyncPlayback)
			trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
		
		m_player->loadFile(trial->getMReference(0)->filepath);
		m_player->setGain(trial->getMReference(0)->gain);
		
		if (timeSyncPlayback)
			m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());
		
		m_renderer->setOrder(trial->getMReference(0)->renderingOrder);

		if (BinauralRenderer::initialiseFromAmbix(File(trial->getMReference(0)->ambixConfig), m_renderer))
			sendMsgToLogWindow("successfully loaded: " + File(trial->getMReference(0)->ambixConfig).getFileName());
		else
			sendMsgToLogWindow("failed to load AmbiX file");

		m_player->play();

		selectReferenceButton.setToggleState(true, dontSendNotification);
	}
	else
	{
		selectReferenceButton.setToggleState(false, dontSendNotification);
	}

	if (buttonThatWasClicked == &selectTConditionAButton)
	{
		if (trial == nullptr)
			return;

		m_player->pause();
		sendMsgToLogWindow("Condition name: " + trial->getTCondition(0)->name);
		
		if (timeSyncPlayback)
			trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
		
		m_player->loadFile(trial->getTCondition(0)->filepath);
		m_player->setGain(trial->getTCondition(0)->gain);
		
		if (timeSyncPlayback)
			m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());
		
		m_renderer->setOrder(trial->getTCondition(0)->renderingOrder);

		if (BinauralRenderer::initialiseFromAmbix(File(trial->getTCondition(0)->ambixConfig), m_renderer))
			sendMsgToLogWindow("successfully loaded: " + File(trial->getTCondition(0)->ambixConfig).getFileName());
		else
			sendMsgToLogWindow("failed to load AmbiX file");

		m_player->play();

		selectTConditionAButton.setToggleState(true, dontSendNotification);
	}
	else
	{
		selectTConditionAButton.setToggleState(false, dontSendNotification);
	}

	if (buttonThatWasClicked == &selectTConditionBButton)
	{
		if (trial == nullptr)
			return;

		m_player->pause();
		sendMsgToLogWindow("Condition name: " + trial->getTCondition(1)->name);
		
		if (timeSyncPlayback)
			trial->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
		
		m_player->loadFile(trial->getTCondition(1)->filepath);
		m_player->setGain(trial->getTCondition(1)->gain);
		
		if (timeSyncPlayback)
			m_player->setPlaybackHeadPosition(trial->getLastPlaybackHeadPosition());
		
		m_renderer->setOrder(trial->getTCondition(1)->renderingOrder);

		if (BinauralRenderer::initialiseFromAmbix(File(trial->getTCondition(1)->ambixConfig), m_renderer))
			sendMsgToLogWindow("successfully loaded: " + File(trial->getTCondition(1)->ambixConfig).getFileName());
		else
			sendMsgToLogWindow("failed to load AmbiX file");
		
		m_player->play();

		selectTConditionBButton.setToggleState(true, dontSendNotification);
	}
	else
	{
		selectTConditionBButton.setToggleState(false, dontSendNotification);
	}

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
	// hide UI
	m_oscTxRx->sendOscMessage("/showUI", (int)0);

	TestTrial* trial = m_testSession->getTrial(m_testSession->getCurrentTrialIndex());
	if (trial != nullptr)
	{
		// send string to display on the screen
		String screenMessage1 = "Trial " + String(m_testSession->getCurrentTrialIndex() + 1) + " of " + String(m_testSession->getNumberOfTrials());
		m_oscTxRx->sendOscMessage("/screenMessages", (String)screenMessage1, (String)trial->getTrialName() + "\n\n" + trial->getTrialInstruction());

		// rating labels
		StringArray ratings = trial->getRatingOptions();
		if (ratings.size() > 0)
		{
			m_oscTxRx->sendOscMessage("/numOfRatingLabels", (int)ratings.size());
			for (int i = 0; i < ratings.size(); ++i)
			{
				m_oscTxRx->sendOscMessage("/ratingLabel", (int)i, (String)ratings[i]);
			}
		}
	}

	// Transport controls
	if (m_player->checkPlaybackStatus())
	{
		m_oscTxRx->sendOscMessage("/buttonState", (String) "play", (int)1);
		m_oscTxRx->sendOscMessage("/buttonState", (String) "stop", (int)0);
	}
	else
	{
		m_oscTxRx->sendOscMessage("/buttonState", (String) "play", (int)0);
		m_oscTxRx->sendOscMessage("/buttonState", (String) "stop", (int)1);
	}

	if (m_player->checkLoopStatus())
	{
		m_oscTxRx->sendOscMessage("/buttonState", (String) "loop", (int)1);
	}
	else
	{
		m_oscTxRx->sendOscMessage("/buttonState", (String) "loop", (int)0);
	}

	// sliders
	if (ratingSliderArray.size() > 0)
	{
		m_oscTxRx->sendOscMessage("/numOfSliders", (int) ratingSliderArray.size());
		for (int i = 0; i < ratingSliderArray.size(); ++i)
		{
			m_oscTxRx->sendOscMessage("/sliderState", (int)i, (float)ratingSliderArray[i]->getValue(), (float)ratingSliderArray[i]->getMinimum(), (float)ratingSliderArray[i]->getMaximum());
		}
	}

	// condition trigger buttons
	if (selectConditionButtonArray.size() > 0)
	{		m_oscTxRx->sendOscMessage("/numOfCondTrigButtons", (int)selectConditionButtonArray.size());
		for (int i = 0; i < ratingSliderArray.size(); ++i)
		{
			m_oscTxRx->sendOscMessage("/condTrigButtonState", (int)i, (int)selectConditionButtonArray[i]->getToggleState());
		}
	}

	// attribute labels
	if (attributeRatingLabels.size() > 0)
	{
		m_oscTxRx->sendOscMessage("/numOfAttributeLabels", (int)attributeRatingLabels.size());
		for (int i = 0; i < ratingSliderArray.size(); ++i)
		{
			m_oscTxRx->sendOscMessage("/attributeLabel", (int)i, (String)attributeRatingLabels[i]->getText());
		}
	}

	// A/B buttons
	if (selectTConditionAButton.isVisible() && selectTConditionBButton.isVisible())
	{
		// send number of A/B buttons
		m_oscTxRx->sendOscMessage("/ABTrigButtonsPresent", (int)1);
		
		// update A / B buttons
		m_oscTxRx->sendOscMessage("/buttonState", (String) "A", (int)selectTConditionAButton.getToggleState());
		m_oscTxRx->sendOscMessage("/buttonState", (String) "B", (int)selectTConditionBButton.getToggleState());
	}
	else
	{
		m_oscTxRx->sendOscMessage("/ABTrigButtonsPresent", (int)0);
	}

	// reference button
	if (selectReferenceButton.isVisible())
	{
		// send number of select reference buttons
		m_oscTxRx->sendOscMessage("/RefTrigButtonPresent", (int)1);

		// update reference button
		m_oscTxRx->sendOscMessage("/buttonState", (String) "reference", (int)selectReferenceButton.getToggleState());
	}
	else
	{
		m_oscTxRx->sendOscMessage("/RefTrigButtonPresent", (int)0);
	}

	// show UI
	m_oscTxRx->sendOscMessage("/showUI", (int)1);
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
			if (selectTConditionAButton.isVisible()) selectTConditionAButton.triggerClick();
		}
		else if (message[0].getString() == "B")
		{
			if (selectTConditionBButton.isVisible()) selectTConditionBButton.triggerClick();
		}
		else if (message[0].getString() == "reference")
		{
			if (selectReferenceButton.isVisible()) selectReferenceButton.triggerClick();
		}
		else if (message[0].getString() == "condA") { if (selectConditionButtonArray[0] != nullptr) selectConditionButtonArray[0]->triggerClick(); }
		else if (message[0].getString() == "condB") { if (selectConditionButtonArray[1] != nullptr) selectConditionButtonArray[1]->triggerClick(); }
		else if (message[0].getString() == "condC") { if (selectConditionButtonArray[2] != nullptr) selectConditionButtonArray[2]->triggerClick(); }
		else if (message[0].getString() == "condD") { if (selectConditionButtonArray[3] != nullptr) selectConditionButtonArray[3]->triggerClick(); }
		else if (message[0].getString() == "condE") { if (selectConditionButtonArray[4] != nullptr) selectConditionButtonArray[4]->triggerClick(); }
		else if (message[0].getString() == "condF") { if (selectConditionButtonArray[5] != nullptr) selectConditionButtonArray[5]->triggerClick(); }
		else if (message[0].getString() == "condG") { if (selectConditionButtonArray[6] != nullptr) selectConditionButtonArray[6]->triggerClick(); }
		else if (message[0].getString() == "condH") { if (selectConditionButtonArray[7] != nullptr) selectConditionButtonArray[7]->triggerClick(); }
		else if (message[0].getString() == "condI") { if (selectConditionButtonArray[8] != nullptr) selectConditionButtonArray[8]->triggerClick(); }
		else if (message[0].getString() == "condJ") { if (selectConditionButtonArray[9] != nullptr) selectConditionButtonArray[9]->triggerClick(); }
		else if (message[0].getString() == "condK") { if (selectConditionButtonArray[10] != nullptr) selectConditionButtonArray[10]->triggerClick(); }

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
