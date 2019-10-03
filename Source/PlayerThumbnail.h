#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PlayerThumbnail : public Component,
						public ChangeListener,
						public MouseListener

{
public:
	PlayerThumbnail();
	~PlayerThumbnail();

	void paint(Graphics&) override;
	void resized() override;

	virtual void changeListenerCallback(ChangeBroadcaster* source) override;
	virtual void mouseDown(const MouseEvent& event) override;

	void clearThumbnail();
	void createThumbnail(File audioFile);

private:


	void paintIfNoFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);
	void paintIfFileLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);

	// creating new format manager for thumbnail painting might slow down thumbnail drawing,
	// it used to share one with the audio player.... ???
	AudioFormatManager formatManager;

	AudioThumbnailCache thumbnailCache;
	AudioThumbnail thumbnail;
};