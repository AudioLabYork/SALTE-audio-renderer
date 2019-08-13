#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class TC_TS26259 : public Component,
					private Button::Listener,
					private Slider::Listener
{
public:
	TC_TS26259();
	~TC_TS26259();

	void paint(Graphics&) override;
	void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void sliderValueChanged(Slider* sliderThatWasChanged) override;


private:
	//OSCSender sender;
	TextButton playButton, stopButton, loopButton;
	TextButton selectAButton, selectBButton;
	TextButton prevTrialButton, nextTrialButton;
	OwnedArray<Slider> ratingSliderArray;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TC_TS26259)
};
