#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRenderer.h"
#include <vector>
#include <random>

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

	TextButton g_chooseStimuliFolder;
	TextButton g_startTest;
	TextButton g_prevTrial, g_nextTrial;
	TextButton g_confirmPointer;

	void indexAudioFiles();
	String audioFilesSrcPath = "*** no path selected ***";
	Array<File> audioFilesArray;
	double totalTimeOfAudioFiles = 0;

	int currentTrialIndex = 0;

	void loadFile();
	void sendMsgToLogWindow(String message);
};

