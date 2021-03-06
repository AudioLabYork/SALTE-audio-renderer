#pragma once

#include "LoudspeakerRenderer.h"
#include "BinauralRenderer.h"
#include "BinauralHeadView.h"

class RendererView
	: public Component
	, public Button::Listener
	, public Timer
	, public ChangeBroadcaster
	, public ChangeListener
	, public BinauralRenderer::Listener
{
public:
	RendererView();

	void init(LoudspeakerRenderer* lsRenderer, BinauralRenderer* binRenderer);
	void deinit();

	void paint(Graphics& g) override;
	void resized() override;
	void changeListenerCallback(ChangeBroadcaster* source) override;
	void buttonClicked(Button* buttonClicked) override;

	void ambixFileLoaded(const File& file) override;
	void sofaFileLoaded(const File& file) override;

	String getCurrentTab();
	void setCurrentTab(String index);

	enum RendererModes
	{
		bypassed,
		loudspeaker,
		binaural
	};

	void setRendererMode(RendererModes targetMode);
	void setTestInProgress(bool inProgress);

	void browseForLsAmbixConfigFile();
	void browseForBinAmbixConfigFile();
	void browseForSofaFile();

	String m_currentLogMessage;

private:
	virtual void timerCallback() override;

	LoudspeakerRenderer* m_lsRenderer;
	BinauralRenderer* m_binRenderer;

	TabbedButtonBar modeSelectTabs;

	Label m_lsAmbixFileLabel;
	TextButton m_lsAmbixFileBrowse;

	TextButton m_loadStandard1OA, m_loadStandard3OA, m_loadStandard5OA;
	Label m_binAmbixFileLabel;
	TextButton m_binAmbixFileBrowse;

	Label m_sofaFileLabel;
	TextButton m_sofaFileBrowse;

	ToggleButton m_enableDualBand;

	String m_sofaFilePath;

	ToggleButton m_enableRotation;
	Label m_rollLabel;
	Label m_pitchLabel;
	Label m_yawLabel;

	BinauralHeadView m_binauralHeadView;

	ToggleButton m_enableMirrorView;
};
