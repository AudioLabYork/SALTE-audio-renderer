#include "TC_TS26259.h"

TC_TS26259::TC_TS26259()
	: m_player(nullptr)
{
	// sender.connect("127.0.0.1", 9000);

	playButton.setButtonText("Play");
	playButton.addListener(this);
	addAndMakeVisible(playButton);

	stopButton.setButtonText("Stop");
	stopButton.addListener(this);
	addAndMakeVisible(stopButton);

	loopButton.setButtonText("Loop");
	loopButton.addListener(this);
	addAndMakeVisible(loopButton);

	selectAButton.setButtonText("A");
	selectAButton.addListener(this);
	addAndMakeVisible(selectAButton);

	selectBButton.setButtonText("B");
	selectBButton.addListener(this);
	addAndMakeVisible(selectBButton);

	prevTrialButton.setButtonText("Previous Trial");
	prevTrialButton.addListener(this);
	addAndMakeVisible(prevTrialButton);

	nextTrialButton.setButtonText("Next Trial");
	nextTrialButton.addListener(this);
	addAndMakeVisible(nextTrialButton);

	// SLIDERS
	for (int i = 0; i < 4; ++i)
	{
		ratingSliderArray.add(new Slider());
		ratingSliderArray[i]->getProperties().set("ratingSlider", true);
		ratingSliderArray[i]->getProperties().set("sliderIndex", i);
		ratingSliderArray[i]->setSliderStyle(Slider::LinearVertical);
		ratingSliderArray[i]->setTextBoxIsEditable(true);
		ratingSliderArray[i]->setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);
		ratingSliderArray[i]->setRange(-3, 3, 0.1);
		ratingSliderArray[i]->setValue(0);
		ratingSliderArray[i]->addListener(this);
		addAndMakeVisible(ratingSliderArray[i]);
	}

	// CONFIGURE THE TEST TRIAL ARRAY
	testTrialArray.add(new TestTrial);
	testTrialArray[0]->setFilepath(0, "D:/ASP_TEST/5OA_RENDER_03.wav");
	testTrialArray[0]->setFilepath(1, "D:/ASP_TEST/5OA_ComplexScene_03_576kbps.wav");

	testTrialArray.add(new TestTrial);
	testTrialArray[1]->setFilepath(0, "D:/ASP_TEST/5OA_RENDER_04.wav");
	testTrialArray[1]->setFilepath(1, "D:/ASP_TEST/5OA_ComplexScene_04_576kbps.wav");
}

TC_TS26259::~TC_TS26259()
{
	// sender.disconnect();
}

void TC_TS26259::init(StimulusPlayer* player)
{
	m_player = player;

	// LOAD THE FIRST TRIAL
	loadTrial(0);
}

void TC_TS26259::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));   // clear the background

	// RECTANGULAR OUTLINE
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

	g.setColour(Colours::white);
	g.setFont(14.0f);

	// HORIZONTAL DASHED LINES
	int linesStartX = 220;
	int linesEndX = 600;
	int linesStartY = 100;
	int linesEndY = 280;
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
	g.setColour(Colours::ghostwhite);
	g.drawDashedLine(line1, dashPattern, 2, 1.0f);
	g.drawDashedLine(line2, dashPattern, 2, 1.0f);
	g.drawDashedLine(line3, dashPattern, 2, 1.0f);
	g.drawDashedLine(line4, dashPattern, 2, 1.0f);
	g.drawDashedLine(line5, dashPattern, 2, 1.0f);
	g.drawDashedLine(line6, dashPattern, 2, 1.0f);

	// RATING SCALE
	int textStartX = linesStartX - 200;
	int textStartY = linesStartY - linesYinterval / 2 - 10;
	int textWidth = 180;
	g.drawText("A is much better than B", textStartX, textStartY, textWidth, 20, Justification::centredRight, true);
	g.drawText("A is better than B", textStartX, textStartY + linesYinterval * 1, textWidth, 20, Justification::centredRight, true);
	g.drawText("A is slightly better than B", textStartX, textStartY + linesYinterval * 2, textWidth, 20, Justification::centredRight, true);
	g.drawText("A is no better or worse than B", textStartX, textStartY + linesYinterval * 3, textWidth, 20, Justification::centredRight, true);
	g.drawText("B is slightly better than A", textStartX, textStartY + linesYinterval * 4, textWidth, 20, Justification::centredRight, true);
	g.drawText("B is better than A", textStartX, textStartY + linesYinterval * 5, textWidth, 20, Justification::centredRight, true);
	g.drawText("B is much better than A", textStartX, textStartY + linesYinterval * 6, textWidth, 20, Justification::centredRight, true);

	// SLIDER LABELS
	g.drawText("Spatial Quality", 210, 40, 100, 25, Justification::centred, true);
	g.drawText("Artifacts", 310, 40, 100, 25, Justification::centred, true);
	g.drawText("Timbral Quality", 410, 40, 100, 25, Justification::centred, true);
	g.drawText("BAQ", 510, 40, 100, 25, Justification::centred, true);

	// TEST INFO
	g.drawText("Trial " + String(currentTrialIndex + 1) + " of: " + String(testTrialArray.size()), 20, 20, 100, 25, Justification::centred, true);
}

void TC_TS26259::resized()
{
	playButton.setBounds(300, 420, 80, 40);
	stopButton.setBounds(400, 420, 80, 40);
	loopButton.setBounds(500, 420, 80, 40);
	selectAButton.setBounds(300, 360, 130, 40);
	selectBButton.setBounds(450, 360, 130, 40);
	prevTrialButton.setBounds(40, 420, 80, 40);
	nextTrialButton.setBounds(140, 420, 80, 40);

	// SLIDERS
	auto sliderWidth = 50;
	auto sliderHeight = 260;
	auto sliderSpacing = 100;
	auto sliderPositionX = 260;
	auto sliderPositionY = 200;

	for (int i = 0; i < 4; ++i)
	{
		ratingSliderArray[i]->setSize(sliderWidth, sliderHeight);
		ratingSliderArray[i]->setCentrePosition(sliderPositionX + i * sliderSpacing, sliderPositionY);
	}
}

void TC_TS26259::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &playButton)
	{
		m_player->setPlaybackHeadPosition(0);
		m_player->play();
	}

	else if (buttonThatWasClicked == &stopButton)
	{
		m_player->stop();
	}

	else if (buttonThatWasClicked == &loopButton)
	{
		if (!testTrialArray[currentTrialIndex]->getLoopingState())
		{
			testTrialArray[currentTrialIndex]->setLooping(true);
			loopButton.setColour(TextButton::buttonColourId, Colours::red);
		}
		else
		{
			testTrialArray[currentTrialIndex]->setLooping(false);
			loopButton.setColour(TextButton::buttonColourId, Component::findColour(TextButton::buttonColourId));
		}

		m_player->loop(testTrialArray[currentTrialIndex]->getLoopingState());
	}

	else if (buttonThatWasClicked == &selectAButton)
	{
		// m_player->stop();
		// testTrialArray[currentTrialIndex]->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
		m_player->loadFile(testTrialArray[currentTrialIndex]->getFilepath(0));
		// m_player->setPlaybackHeadPosition(testTrialArray[currentTrialIndex]->getLastPlaybackHeadPosition());
		m_player->play();
	}

	else if (buttonThatWasClicked == &selectBButton)
	{
		// m_player->stop();
		// testTrialArray[currentTrialIndex]->setLastPlaybackHeadPosition((m_player->getPlaybackHeadPosition()));
		m_player->loadFile(testTrialArray[currentTrialIndex]->getFilepath(1));
		// m_player->setPlaybackHeadPosition(testTrialArray[currentTrialIndex]->getLastPlaybackHeadPosition());
		m_player->play();
	}
	
	else if (buttonThatWasClicked == &prevTrialButton)
	{
		if (currentTrialIndex > 0)
		{
			currentTrialIndex--;
			loadTrial(currentTrialIndex);
		}
	}

	else if (buttonThatWasClicked == &nextTrialButton)
	{
		if (currentTrialIndex < testTrialArray.size() - 1)
		{
			currentTrialIndex++;
			loadTrial(currentTrialIndex);
		}
	}

	repaint();
}

void TC_TS26259::sliderValueChanged(Slider* sliderThatWasChanged)
{
	bool rateSampleSliderChanged = sliderThatWasChanged->getProperties()["ratingSlider"];

	if (rateSampleSliderChanged)
	{
		int sliderIndex = sliderThatWasChanged->getProperties()["sliderIndex"];
	}
}

void TC_TS26259::loadTrial(int trialIndex)
{
	if (m_player == nullptr)
	{
		// m_player needs to be initialized
		jassertfalse;
		return;
	}

	currentTrialIndex = trialIndex;
	m_player->loadFile(testTrialArray[currentTrialIndex]->getFilepath(0));
	
	loopButton.setColour(TextButton::buttonColourId, Colours::red);
	m_player->loop(testTrialArray[currentTrialIndex]->getLoopingState());
}