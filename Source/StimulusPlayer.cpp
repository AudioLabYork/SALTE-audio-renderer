#include "../JuceLibraryCode/JuceHeader.h"
#include "StimulusPlayer.h"

StimulusPlayer::StimulusPlayer() : readAheadThread("transport read ahead")
{
    formatManager.registerBasicFormats();
    readAheadThread.startThread(3);
    
    //EDITOR
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open file... (DOES NOTHING ATM)");
    openButton.addListener(this);
    
    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.setColour(TextButton::buttonColourId, Colours::green);
    playButton.addListener(this);
    
    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.setColour(TextButton::buttonColourId, Colours::red);
    stopButton.addListener(this);
    
    loadedFileName.setText("Loaded file:", dontSendNotification);
    addAndMakeVisible(loadedFileName);
    
    playbackHeadPosition.setText("Time:", dontSendNotification);
    addAndMakeVisible(playbackHeadPosition);
    
    startTimer(30);
    
}

StimulusPlayer::~StimulusPlayer()
{
    transportSource.setSource(nullptr);
}

void StimulusPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}
void StimulusPlayer::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (currentAudioFileSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    transportSource.getNextAudioBlock (bufferToFill);
	// ambisonic rotation ar.process(bufferToFill);
}
void StimulusPlayer::releaseResources()
{
    transportSource.releaseResources();
}

void StimulusPlayer::paint (Graphics& g)
{
    // BACKGROUND
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    // RECTANGULAR OUTLINE
    g.setColour(Colours::black);
    g.drawRect(getLocalBounds(), 1);
    
    // INNER RECTANGLES
    Rectangle<int> tcRect(10, 10, 250, 150);        // manual transport control
    Rectangle<int> dispRect(270, 10, 520, 150);        // display state
    Rectangle<int> wfRect(10, 170, 780, 80);        // waveform
    Rectangle<int> trgRect(10, 260, 780, 115);        // triggers
    
    // DRAW RECTANGLES
    g.setColour(Colours::black);
    g.drawRect(tcRect, 1);
    g.drawRect(dispRect, 1);
    g.drawRect(wfRect, 1);
    g.drawRect(trgRect, 1);
    
    // TEXT
    g.setFont(Font(14.0f));
    g.setColour(Colours::white);
}

void StimulusPlayer::resized()
{
    openButton.setBounds(20, 20, 230, 25);
    playButton.setBounds(20, 55, 230, 25);
    stopButton.setBounds(20, 90, 230, 25);
    
    loadedFileName.setBounds(280, 20, 500, 25);
    playbackHeadPosition.setBounds(280, 45, 500, 25);
        
    if (numberOfStimuli > 0)
    {
        int buttonWidth = 57;
        int buttonHeight = 22;
        int buttonListPositionX = 20 + buttonWidth / 2;
        int buttonListPositionY = 270 + buttonHeight / 2;
        
        for (int i = 0; i < triggerStimuliButtonArray.size(); ++i)
        {
            triggerStimuliButtonArray[i]->setSize(buttonWidth, buttonHeight);
            triggerStimuliButtonArray[i]->setCentrePosition(buttonListPositionX + i % 12 * (buttonWidth + 5),
                                                            buttonListPositionY + floor(i / 12) * (buttonHeight + 5));
        }
    }
}

void StimulusPlayer::createFilePathList(String configFilePath)
{
    File fileToLoad = File(configFilePath);
    StringArray loadedData;
    loadedData.clear();
    loadedData.addLines(fileToLoad.loadFileAsString());
    if (loadedData[0].startsWith("#ASP#Config#File#"))
    {
        filePathList.clear();
        int header = 10; // number of header lines
        for (int i = header; i < loadedData.size(); ++i)
        {
            StringArray tokens;
            tokens.addTokens(loadedData[i], ",", "\"");
            if (tokens[3].length() != 0)
            {
                filePathList.set(i, tokens[2] + "/" + tokens[3]); // concatenate file path + file name and add the full path to the file path list
                fileIdList.set(i, tokens[0] + tokens[1]); // add the file-id to the file-id list
                
                // output log
                sendMsgToLogWindow(tokens[0] + tokens[1] + ": "+ tokens[3] + " added.");
            }
        }
        
        // load the first file from the list
        loadFileIntoTransport(File(filePathList[0]));
    }
    else
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon, "Invalid config file.", fileToLoad.getFullPathName());
    }
}


void StimulusPlayer::loadFileIntoTransport(const File& audioFile)
{
    // unload the previous file source and delete it..
    transportSource.stop();
    transportSource.setSource(nullptr);
    currentAudioFileSource = nullptr;
    
    AudioFormatReader* reader = formatManager.createReaderFor(audioFile);
    currentlyLoadedFile = audioFile;
    
    if (reader != nullptr)
    {
        currentAudioFileSource = new AudioFormatReaderSource(reader, true);
        
        // loading into transport source using a separate thread comes from https://github.com/jonathonracz/AudioFilePlayerPlugin
        // ..and plug it into our transport source
        transportSource.setSource(
                                  currentAudioFileSource,
                                  32768,                  // tells it to buffer this many samples ahead
                                  &readAheadThread,       // this is the background thread to use for reading-ahead
                                  reader->sampleRate,     // allows for sample rate correction
                                  reader->numChannels);    // the maximum number of channels that may need to be played
        // update GUI label
        loadedFileName.setText("Loaded file: " + currentlyLoadedFile.getFileName(), dontSendNotification);
        
        // send message to the main log window
        sendMsgToLogWindow("Loaded: " + audioFile.getFileName());
        sendMsgToLogWindow(String(reader->numChannels) + "," +
                           String(reader->bitsPerSample) + "," +
                           String(reader->sampleRate) + "," +
                           String(reader->lengthInSamples) + "," +
                           String(reader->lengthInSamples / reader->sampleRate));
    }
}

void StimulusPlayer::sendMsgToLogWindow(String message)
{
    // is it safe? (1/2)
    currentMessage += message + "\n";
    sendChangeMessage();  // broadcast change message to inform and update the editor
}

void StimulusPlayer::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &openButton)
    {
        // browseForConfigFile();
    }
    else if (buttonThatWasClicked == &playButton)
    {
        transportSource.setPosition(0);
        transportSource.start();
    }
    else if (buttonThatWasClicked == &stopButton)
    {
        transportSource.stop();
    }
    else if (buttonThatWasClicked->getProperties()["triggerStimuliButton"])
    {
        int buttonIndex = buttonThatWasClicked->getProperties()["buttonIndex"];
        loadFileIntoTransport(File(filePathList[buttonIndex]));
        transportSource.setPosition(0);
        transportSource.start();
    }
    repaint();
}

void StimulusPlayer::timerCallback()
{
    double currentPosition = transportSource.getCurrentPosition();
    double lengthInSeconds = transportSource.getLengthInSeconds();
    
    // update diplayed times in GUI
    playbackHeadPosition.setText("Time: " + returnHHMMSS(currentPosition) + " / " + returnHHMMSS(lengthInSeconds), dontSendNotification);
}

String StimulusPlayer::returnHHMMSS(double lengthInSeconds)
{
    int hours = (int)lengthInSeconds / (60 * 60);
    int minutes = ((int)lengthInSeconds / 60) % 60;
    int seconds = ((int)lengthInSeconds) % 60;
    int millis = floor((lengthInSeconds - floor(lengthInSeconds)) * 100);
    
    String output = String(hours).paddedLeft('0', 2) + ":" +
    String(minutes).paddedLeft('0', 2) + ":" +
    String(seconds).paddedLeft('0', 2) + "." +
    String(millis).paddedLeft('0', 2);
    return output;
}

void StimulusPlayer::createStimuliTriggerButtons()
{
        for (int i = 0; i < numberOfStimuli; ++i)
    {
        triggerStimuliButtonArray.add(new TextButton());
        triggerStimuliButtonArray[i]->getProperties().set("triggerStimuliButton", true);
        triggerStimuliButtonArray[i]->getProperties().set("buttonIndex", i);
        triggerStimuliButtonArray[i]->setButtonText(fileIdList[i]);
        triggerStimuliButtonArray[i]->addListener(this);
        addAndMakeVisible(triggerStimuliButtonArray[i]);
    }
    resized();
}
