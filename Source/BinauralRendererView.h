#pragma once

#include "BinauralRenderer.h"
#include "BinauralHeadView.h"

class BinauralRendererView
	: public Component
	, public Button::Listener
	, public Timer
	, public ChangeBroadcaster
	, public BinauralRenderer::Listener
{
public:
	BinauralRendererView();

	void init(BinauralRenderer* renderer);
	void deinit();

	void paint(Graphics& g) override;
	void resized() override;

	void buttonClicked(Button* buttonClicked) override;

	void ambixFileLoaded(const File& file) override;
	void sofaFileLoaded(const File& file) override;

	void setTestInProgress(bool inProgress);

	void browseForAmbixConfigFile();
	void browseForSofaFile();

	String m_currentLogMessage;

private:
	virtual void timerCallback() override;

	BinauralRenderer* m_renderer;

	ToggleButton m_enableRenderer;

	Label m_ambixFileLabel;
	TextButton m_ambixFileBrowse;

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
