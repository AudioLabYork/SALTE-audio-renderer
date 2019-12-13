#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRenderer.h"
#include <vector>
#include <random>

class AuditoryLocalisation	:	public Component
							//,	public OSCReceiver
							,	public OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
							,	private Button::Listener
							,	private ChangeListener
							,	public ChangeBroadcaster
							,	public Timer
{
public:
	AuditoryLocalisation();
	~AuditoryLocalisation();

	void init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRenderer* renderer);

	void paint(Graphics& g) override;
	void resized() override;
	void buttonClicked(Button* buttonThatWasClicked) override;
	void oscMessageReceived(const OSCMessage& message) override;
	void oscBundleReceived(const OSCBundle& bundle) override;
	void changeListenerCallback(ChangeBroadcaster* source) override;
	void timerCallback() override;

	String currentMessage;
private:
	OscTransceiver* m_oscTxRx;
	StimulusPlayer* m_player;
	BinauralRenderer* m_renderer;

	TextButton m_chooseStimuliFolder;
	TextButton m_startTest;
	TextButton m_prevTrial, m_nextTrial;
	TextButton m_confirmPointer;

	void selectSrcPath();
	void indexAudioFiles();
	File audioFilesDir;
	Array<File> audioFilesArray;
	double totalTimeOfAudioFiles = 0;

	int currentTrialIndex = 0;

	void loadFile();
	void sendMsgToLogWindow(String message);

	// OSC logging
	void processOscMessage(const OSCMessage& message);
	void saveLog();
	int port = 9000;
	double activationTime = 0.0f;
	TextButton m_saveLogButton;
	Label messageCounter;
	StringArray oscMessageList;

	// settings
	void initSettings();
	void loadSettings();
	void saveSettings();
	ApplicationProperties TestSessionFormSettings;
};

