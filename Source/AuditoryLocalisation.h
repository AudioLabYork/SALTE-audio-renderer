#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRenderer.h"

class AuditoryLocalisation	:	public Component
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

	String getAudioFilesDir();
	void setAudioFilesDir(String filePath);
private:
	OscTransceiver* m_oscTxRx;
	StimulusPlayer* m_player;
	BinauralRenderer* m_renderer;

	TextButton m_chooseStimuliFolder;

	Label m_calStimuliName, m_testStimuliName;
	Label m_calStimuliReps, m_testStimuliReps;

	ToggleButton m_autoNextTrial, m_loopStimulus;
	TextButton m_prevTrial, m_nextTrial;
	TextButton m_startStopTest;
	TextButton m_saveLogButton;
	Label m_messageCounter;
	TextEditor m_lastMessage, m_logHeaderTE;



	Label m_distance;
	ToggleButton m_horizonLocked, m_meshHorizonOn, m_pointerOn;
	void setHorizon();

	void selectSrcPath();
	void indexAudioFiles();
	String returnHHMMSS(double lengthInSeconds);
	File audioFilesDir;
	Array<File> audioFilesArray;
	AudioFormatManager formatManager;
	double totalTimeOfAudioFiles = 0;

	int m_currentTrialIndex = 0;
	void changeTrial(int newTrialIndex);
	void playStimulus();
	void playBeep(int beep_type);
	void sendMsgToLogWindow(String message);


	// OSC logging
	void processOscMessage(const OSCMessage& message);
	void saveLog();
	double m_activationTime = 0.0f;
	StringArray oscMessageList;
	String m_logHeader = "salte_time,trial_index,stimulus,osc_pattern,ml_time,et_az,et_el,et_rot,et_dist,et_conf,cal_status,leye_conf,reye_conf,ptr_az,ptr_el,target_az,target_el\n";
};
