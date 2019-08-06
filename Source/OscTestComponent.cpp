#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTestComponent.h"

OscTestComponent::OscTestComponent()
{
	sender.connect("127.0.0.1", 9001);

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
}

void OscTestComponent::buttonClicked(Button* buttonThatWasClicked)
{
	if (buttonThatWasClicked == &btnA)
	{
		sender.send("/transport/", (String) "play");
	}
	
	if (buttonThatWasClicked == &btnB)
	{

	}
	
	if (buttonThatWasClicked == &btnC)
	{

	}

	if (buttonThatWasClicked == &btnD)
	{

	}
}
