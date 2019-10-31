#include "AuditoryLocalisation.h"

AuditoryLocalisation::AuditoryLocalisation()
					: m_oscTxRx(nullptr)
					, m_player(nullptr)
					, m_renderer(nullptr)
{

}

AuditoryLocalisation::~AuditoryLocalisation()
{

}

void AuditoryLocalisation::init(StimulusPlayer* player, BinauralRenderer* renderer)
{
	m_renderer = renderer;
	m_player = player;
	m_player->addChangeListener(this);
}

void AuditoryLocalisation::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

	g.setColour(Colours::white);
	g.drawText("Localisation", 75, 75, 250, 25, Justification::centred);
}
void AuditoryLocalisation::resized()
{

}
void AuditoryLocalisation::buttonClicked(Button* buttonThatWasClicked)
{

}

void AuditoryLocalisation::oscMessageReceived(const OSCMessage& message)
{

}

void AuditoryLocalisation::changeListenerCallback(ChangeBroadcaster* source)
{

}

void AuditoryLocalisation::sendMsgToLogWindow(String message)
{
	currentMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
}