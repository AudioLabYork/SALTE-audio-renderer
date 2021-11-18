#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRenderer.h"
#include "TestSession.h"

class MixedMethodsComponent
	: public Component
	, public OSCReceiver
	, public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>
	, private Button::Listener
	, private Slider::Listener
	, private ChangeListener
	, public ChangeBroadcaster
	, private Timer
{
public:
	//==============================================================================
	MixedMethodsComponent();
	~MixedMethodsComponent();

	void init(OscTransceiver* oscTxRx, TestSession* testSession, StimulusPlayer* player, BinauralRenderer* renderer);

	void loadTestSession();
	void setLocalIpAddress(String ip);

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
	void sliderDragStarted(Slider* sliderThatHasBeenStartedDragging) override;
	void oscMessageReceived(const OSCMessage& message) override;
	void changeListenerCallback(ChangeBroadcaster* source) override;

	// void triggerConditionPlayback(int buttonIndex);
	void updateRemoteInterface(bool testIsOn);
	String localIpAddress;

	bool loadCondition(String type, int i);
	void timerCallback() override;
	bool m_delayStart = false;
	int64 m_time = 0;
	bool m_continuousPlayback = true;
	String m_lastStimulusPath = "";
	String m_lastAmbixConfig = "";

	TextButton prevTrialButton, nextTrialButton, endTestButton;

	TextButton selectReferenceButton;
	TextButton selectTConditionAButton, selectTConditionBButton; // used for TS26259
	OwnedArray<TextButton> selectConditionButtonArray;
	OwnedArray<Slider> ratingSliderArray;
	OwnedArray<Label> ratingReadouts;
	OwnedArray<Label> attributeRatingLabels; // used for TS26259

	// references to other SALTE Renderer objects
	OscTransceiver* m_oscTxRx;
	StimulusPlayer* m_player;
	BinauralRenderer* m_renderer;
	TestSession* m_testSession;

	int leftBorder;
	int rightBorder;
	int topBorder;
	int bottomBorder;

	juce::Rectangle<int> testSpace;
	juce::Rectangle<int> testArea;

	ListenerList<Listener> mushraTestListeners;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixedMethodsComponent)
};
