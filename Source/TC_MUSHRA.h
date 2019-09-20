#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRenderer.h"
#include "TestSession.h"

class MushraComponent : public Component,
						public OSCReceiver,
						public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>,
						private Button::Listener,
						private Slider::Listener,
						private ChangeListener,
						public ChangeBroadcaster
{
public:
	//==============================================================================
	MushraComponent();
	~MushraComponent();

	void init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRenderer* renderer);
	void loadTestSession(TestSession* testSession);

	class Listener
	{
	public:
		virtual ~Listener() = default;
		virtual void testCompleted() = 0;
	};

	void addListener(Listener* newListener);
	void removeListener(Listener* listener);

	void sendMsgToLogWindow(String message);

	// log window message
	String currentMessage;

private:
	void loadTrial(int trialIndex);
	void paint(Graphics&) override;
	void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void sliderValueChanged(Slider* sliderThatWasChanged) override;
	void oscMessageReceived(const OSCMessage& message) override;
	void changeListenerCallback(ChangeBroadcaster* source) override;

	void updateRemoteInterface();

	TextButton prevTrialButton, nextTrialButton, endTestButton;
	
	TextButton selectReferenceButton;
	TextButton selectTConditionAButton, selectTConditionBButton; // used for TS26259
	OwnedArray<TextButton> selectConditionButtonArray;
	OwnedArray<Slider> ratingSliderArray;
	OwnedArray<Label> ratingReadouts;
	OwnedArray<Label> attributeRatingLabels; // used for TS26259

	bool timeSyncPlayback = true;

	// references to other SALTE Renderer objects
	OscTransceiver* m_oscTxRx;
	StimulusPlayer* m_player;
	BinauralRenderer* m_renderer;
	TestSession* m_testSession;

	int leftBorder;
	int rightBorder;
	int topBorder;
	int bottomBorder;
	int ratingLabelsTextWidth;

	juce::Rectangle<int> testSpace;
	juce::Rectangle<int> testArea;

	ListenerList<Listener> mushraTestListeners;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MushraComponent)
};
