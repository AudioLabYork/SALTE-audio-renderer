#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "OscTransceiver.h"
#include "StimulusPlayer.h"
#include "BinauralRenderer.h"

class AuditoryLocalisation	:	public Component
							,	public ChangeBroadcaster
{
public:
	AuditoryLocalisation();
	~AuditoryLocalisation();

	void init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRenderer* renderer);
	void paint(Graphics& g) override;
	void resized() override;

	String getAudioFilesDir();
	void setAudioFilesDir(String filePath);

	String currentMessage;
private:
	OscTransceiver* m_oscTxRx;
	StimulusPlayer* m_player;
	BinauralRenderer* m_renderer;

	File audioFilesDir;
};

