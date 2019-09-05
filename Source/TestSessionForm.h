#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SubjectData.h"
#include "TestSession.h"

class TestSessionForm
	: public Component
	, public Timer
	, public Button::Listener
{
public:
	TestSessionForm();
	~TestSessionForm();

	void init(TestSession* session);
	void reset();

	class Listener
	{
	public:
		virtual ~Listener() = default;
		virtual void formCompleted() = 0;
	};

	void addListener(Listener* newListener);
	void removeListener(Listener* listener);

private:
	void paint(Graphics& g) override;
	void resized() override;
	void timerCallback() override;
	void buttonClicked(Button* button) override;

	TestSession* m_session;
	Label m_labelSession;

	Label m_labelSessionName;
	Label m_labelSessionFile;
	Label m_labelExportFile;
	TextEditor m_editSessionName;
	TextButton m_btnSessionFile;
	TextButton m_btnExportFile;

	File m_sessionFile;
	File m_exportFile;

	ToggleButton m_btnStickySession;
	bool m_shouldSaveSessionSettings;

	Label m_labelSubject;

	Label m_labelName;
	Label m_labelAge;
	Label m_labelGender;
	TextEditor m_editName;
	TextEditor m_editAge;
	TextEditor m_editGender;
	ToggleButton m_btnAnon;
	ToggleButton m_btnAgree;
	TextButton m_btnBegin;

	ListenerList<Listener> testSessionListeners;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestSessionForm)
};
