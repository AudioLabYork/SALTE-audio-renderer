#pragma once

#include "BinauralRenderer.h"
#include "BinauralHeadView.h"

enum eOrders
{
	k1stOrder,
	k2ndOrder,
	k3rdOrder,
	k4thOrder,
	k5thOrder,
	k6thOrder,
	k7thOrder
};

static StringArray orderChoices = { "1st Order",
									"2nd Order",
									"3rd Order",
									"4th Order",
									"5th Order",
									"6th Order",
									"7th Order" };
									
class BinauralRendererView
	: public Component
	, public Button::Listener
	, public ComboBox::Listener
	, public Timer
	, public ChangeBroadcaster
{
public:
	BinauralRendererView();
	
	void init(BinauralRenderer* renderer);
	void deinit();

	void paint(Graphics& g) override;
	void resized() override;
	
	void buttonClicked(Button* buttonClicked) override;
	void comboBoxChanged(ComboBox* comboBoxChanged) override;

	void setTestInProgress(bool inProgress);

	void changeComboBox(int order);
	
	void sendMsgToLogWindow(const String& message);
	
	void browseForAmbixConfigFile();
	void browseForSofaFile();

	void loadStandardHRTF();

	String m_currentLogMessage;
	
private:
	virtual void timerCallback() override;

	BinauralRenderer* m_renderer;
	
	TextButton m_ambixFileBrowse;
	ToggleButton m_useSofa;
	
	ComboBox m_orderSelect;
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
