#include "PlayerThumbnail.h"

PlayerThumbnail::PlayerThumbnail() :	thumbnailCache(10), // maxNumThumbsToStore parameter lets you specify how many previews should be kept in memory at once.
										thumbnail(128, formatManager, thumbnailCache)
{
	formatManager.registerBasicFormats();
	thumbnail.addChangeListener(this);
}

PlayerThumbnail::~PlayerThumbnail()
{

}

void PlayerThumbnail::paint(Graphics& g)
{
	Rectangle<int> wfRect(getLocalBounds());

	// PAINT WAVEFORM
	if (thumbnail.getNumChannels() == 0)
		paintIfNoFileLoaded(g, wfRect);
	else
		paintIfFileLoaded(g, wfRect);
}

void PlayerThumbnail::resized()
{

}

void PlayerThumbnail::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == &thumbnail)
		repaint();
}

void PlayerThumbnail::mouseDown(const MouseEvent& event)
{
	//sendChangeMessage();
}

void PlayerThumbnail::clearThumbnail()
{
	thumbnail.setSource(nullptr);
}

void PlayerThumbnail::createThumbnail(File audioFile)
{
	thumbnail.setSource(new FileInputSource(audioFile));
}

void PlayerThumbnail::setPlaybackCursor(double value)
{
	playbackCursorPosition = value;
	repaint();
}

void PlayerThumbnail::setPlaybackOffsets(double beg, double end)
{
	begOffset = beg;
	endOffset = end;
	repaint();
}

void PlayerThumbnail::paintIfNoFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds)
{
	g.setColour(Colours::darkgrey);
	g.fillRect(thumbnailBounds);
	g.setColour(Colours::white);
	g.drawFittedText("No File Loaded", thumbnailBounds, Justification::centred, 1);
}

void PlayerThumbnail::paintIfFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds)
{
	g.setColour(Colours::white);
	g.fillRect(thumbnailBounds);

	g.setColour(Colours::grey);
	Rectangle<int> begOffsetRect = thumbnailBounds;
	Rectangle<int> endOffsetRect = thumbnailBounds;
	begOffsetRect.removeFromRight((int)thumbnailBounds.getWidth() * (1 - begOffset));
	endOffsetRect.removeFromLeft((int)thumbnailBounds.getWidth() * (1 - endOffset));
	g.fillRect(begOffsetRect);
	g.fillRect(endOffsetRect);

	g.setColour(Colours::red);                                     // [8]
	thumbnail.drawChannel(g,                                      // [9]
		thumbnailBounds,
		0.0,                                    // start time
		thumbnail.getTotalLength(),             // end time
		0,										// channel number
		1.0f);                                  // vertical zoom

	g.setColour(Colours::black);
	Rectangle<int> cursorRect;
	cursorRect.setHeight(thumbnailBounds.getHeight());
	cursorRect.setWidth(3);

	int cursorPos = (int)thumbnailBounds.getWidth() * playbackCursorPosition;
	cursorRect.setCentre(cursorPos, thumbnailBounds.getCentreY());
	g.fillRect(cursorRect);

}

