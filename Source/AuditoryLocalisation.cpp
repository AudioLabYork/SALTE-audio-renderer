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

void AuditoryLocalisation::init(OscTransceiver* oscTxRx, StimulusPlayer* player, BinauralRenderer* renderer)
{
	m_renderer = renderer;
	m_player = player;
	m_oscTxRx = oscTxRx;
}

void AuditoryLocalisation::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	g.setColour(Colours::black);
	g.drawRect(getLocalBounds(), 1);

	g.setColour(Colours::white);
	g.setFont(20.0f);
	g.drawText("The localisation component is under development.", getLocalBounds(), Justification::centred);
}

void AuditoryLocalisation::resized()
{

}

String AuditoryLocalisation::getAudioFilesDir()
{
	return audioFilesDir.getFullPathName();
}

void AuditoryLocalisation::setAudioFilesDir(String filePath)
{
	if (File(filePath).exists())
	{
		audioFilesDir = filePath;
	}
}