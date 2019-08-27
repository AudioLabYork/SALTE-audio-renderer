#include "TC_OSC.h"

OscTestComponent::OscTestComponent()
{
	sender.connect("127.0.0.1", 9000);

	btnA.setButtonText("A");
	btnA.addListener(this);
	addAndMakeVisible(btnA);

	btnB.setButtonText("B");
	btnB.addListener(this);
	addAndMakeVisible(btnB);

	btnC.setButtonText("C");
	btnC.addListener(this);
	addAndMakeVisible(btnC);

	btnD.setButtonText("D");
	btnD.addListener(this);
	addAndMakeVisible(btnD);

	close.setButtonText("Close Test");
	close.addListener(this);
	addAndMakeVisible(close);
}

OscTestComponent::~OscTestComponent()
{
	sender.disconnect();
}

void OscTestComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background

	// RECTANGULAR OUTLINE
	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

    g.setColour (Colours::white);
    g.setFont (14.0f);
    g.drawText ("OscTestComponent", getLocalBounds(),
                Justification::centred, true);   // draw some placeholder text
}

void OscTestComponent::resized()
{
	btnA.setBounds(40, 40, 40, 40);
	btnB.setBounds(40, 100, 40, 40);
	btnC.setBounds(40, 160, 40, 40);
	btnD.setBounds(40, 220, 40, 40);

	close.setBounds(getWidth() - 100, 0, 100, 30);
}

void OscTestComponent::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &btnA)
	{
		sender.send("/transport/", (String) "play");
	}
	else if (buttonThatWasClicked == &btnB)
	{
	}
	else if (buttonThatWasClicked == &btnC)
	{
	}
	else if (buttonThatWasClicked == &btnD)
	{
	}
	else if (buttonThatWasClicked == &close)
	{
		setVisible(false);
	}
}
