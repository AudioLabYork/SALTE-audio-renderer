#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRenderer.h"

class AuditoryLocalisation : public Component,
								public OSCReceiver,
								public OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>,
								private Button::Listener,
								private ChangeListener,
								public ChangeBroadcaster
{
public:
	AuditoryLocalisation();
	~AuditoryLocalisation();

	void init(StimulusPlayer* player, BinauralRenderer* renderer);

	void paint(Graphics& g) override;
	void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void oscMessageReceived(const OSCMessage& message) override;
	void changeListenerCallback(ChangeBroadcaster* source) override;

	String currentMessage;
private:
	OscTransceiver* m_oscTxRx;
	StimulusPlayer* m_player;
	BinauralRenderer* m_renderer;

	void sendMsgToLogWindow(String message);
};

