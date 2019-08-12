#pragma once

#include "BinauralRenderer.h"
#include "SOFAReader.h"

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
	void comboBoxChanged(ComboBox* comboBoxChanged);
	
	void sendMsgToLogWindow(const String& message);
	
	void browseForAmbixConfigFile();
	void browseForSofaFile();
	
	void loadAmbixConfigFile(const File& file);
	void loadSofaFile(const File& file);
	void loadStandardHRTF();

	String m_currentLogMessage;
	
private:
	BinauralRenderer* m_renderer;
	
	TextButton m_ambixFileBrowse;
	ToggleButton m_useSofa;
	
	ComboBox m_orderSelect;
	TextButton m_sofaFileBrowse;
	
	String m_sofaFilePath;
	
	ToggleButton m_enableRotation;
	Label m_rollLabel;
	Label m_pitchLabel;
	Label m_yawLabel;
	
	virtual void timerCallback() override;
};
