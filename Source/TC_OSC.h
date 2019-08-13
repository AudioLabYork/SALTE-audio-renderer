#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class OscTestComponent    : public Component,
							private Button::Listener
{
public:
    OscTestComponent();
    ~OscTestComponent();

    void paint (Graphics&) override;
    void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;


private:
	OSCSender sender;
	TextButton btnA, btnB, btnC, btnD;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscTestComponent)
};
