#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PlayerThumbnail : public Component,
						public ChangeListener,
						public ChangeBroadcaster
{
public:
	PlayerThumbnail();
	~PlayerThumbnail();

	void paint(Graphics&) override;
	void resized() override;

	void changeListenerCallback(ChangeBroadcaster* source) override;
	void mouseDown(const MouseEvent& event) override;

	void clearThumbnail();
	void createThumbnail(File audioFile);

	void setPlaybackCursor(double value);
	void setPlaybackOffsets(double beg, double end);

private:


	void paintIfNoFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);
	void paintIfFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);
	// void paintPlaybackCursor(Graphics& g, const Rectangle<int>& thumbnailBounds);

	double playbackCursorPosition = 0;
	double begOffset = 0, endOffset = 0;

	// creating new format manager for thumbnail painting might slow down thumbnail drawing,
	// it used to share one with the audio player.... ???
	AudioFormatManager formatManager;

	AudioThumbnailCache thumbnailCache;
	AudioThumbnail thumbnail;
};
