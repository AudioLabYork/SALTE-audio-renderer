/*
  ==============================================================================

    StimulusPlayer.cpp
    Created: 9 Jul 2019 11:33:50am
    Author:  TR

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "StimulusPlayer.h"

//==============================================================================
StimulusPlayer::StimulusPlayer() : readAheadThread("transport read ahead")
{
    formatManager.registerBasicFormats();
    readAheadThread.startThread(3);
    
    // createFilePathList("D:/ASP_TEST/aspConfigFile.csv"); // load this config file at startup
    
    // OSC sender and receiver connect
    DBG("Connecting OSC ...");
    remoteInterfaceTxRx.connectSender("127.0.0.1", 6000);
    remoteInterfaceTxRx.connectReceiver(9000);
    remoteInterfaceTxRx.addListener(this);
    
    //EDITOR
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open config file...");
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
    
    addAndMakeVisible(logWindow);
    logWindow.setMultiLine(true);
    logWindow.setReadOnly(true);
    logWindow.setCaretVisible(false);
    logWindow.setScrollbarsShown(true);
    
    //setSize (800, 600);
    
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
    Rectangle<int> wfRect(10, 170, 780, 120);        // waveform
    Rectangle<int> trgRect(10, 300, 780, 150);        // triggers
    Rectangle<int> cnslRect(10, 460, 520, 130);        // console
    Rectangle<int> oscRect(540, 460, 250, 130);        // osc status
    
    // DRAW RECTANGLES
    g.setColour(Colours::black);
    g.drawRect(tcRect, 1);
    g.drawRect(dispRect, 1);
    g.drawRect(wfRect, 1);
    g.drawRect(trgRect, 1);
    g.drawRect(cnslRect, 1);
    g.drawRect(oscRect, 1);
    
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
    
    logWindow.setBounds(10, 460, 520, 130);
    
    if (numberOfStimuli > 0)
    {
        int buttonWidth = 57;
        int buttonHeight = 22;
        int sliderPositionX = 20 + buttonWidth / 2;
        int sliderPositionY = 310 + buttonHeight / 2;
        
        for (int i = 0; i < triggerStimuliButtonArray.size(); ++i)
        {
            triggerStimuliButtonArray[i]->setSize(buttonWidth, buttonHeight);
            triggerStimuliButtonArray[i]->setCentrePosition(sliderPositionX + i % 12 * (buttonWidth + 5), sliderPositionY + floor(i / 12) * (buttonHeight + 5));
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
    logWindowMessage += message + "\n";
    sendChangeMessage();  // broadcast change message to inform and update the editor
}


// OSC
void StimulusPlayer::oscMessageReceived(const OSCMessage& message)
{
    sendMsgToLogWindow("OSC RECEIVED ");
    
    
    if (message.size() == 1 && message.getAddressPattern() == "/stimulus" && message[0].isInt32())
    {
        loadFileIntoTransport(File(filePathList[message[0].getInt32()]));
    }
    
    if (message.size() == 1 && message.getAddressPattern() == "/transport" && message[0].isString())
    {
        if (message[0].getString() == "play")
        {
            transportSource.setPosition(0);
            transportSource.start();
        }
        
        if (message[0].getString() == "stop")
        {
            transportSource.stop();
        }
        
    }
    
    
    //logWindowMessage += String("Loaded: ") + message[0].getString() + "\n";
    //sendChangeMessage(); // broadcast change message to inform and update the editor
    
    //
    //DBG(message.getAddressPattern().toString() + message[0].getString());
    
    //if (message.size() == 1 && message.getAddressPattern() == "/lastmarker/number/str" && message[0].isString())
    //{
    //    lastMarkerIndex = message[0].getString().getIntValue();
    //    DBG("Last marker index: " + String(lastMarkerIndex));
    //}
    //if (message.size() == 1 && message.getAddressPattern() == "/time" && message[0].isFloat32() && message[0].getFloat32() != 0)
    //{
    //    currentPosition = message[0].getFloat32();
    //    markerTimeArray.set(markerIndex, currentPosition);
    //    DBG("Last marker position: " + String(currentPosition));
    //}
    
    //if (message.size() == 1 && message.getAddressPattern() == "/track/number/str" && message[0].isString())
    //{
    //    lastTrackIndex = message[0].getString().getIntValue();
    //    DBG("Last track index: " + String(lastTrackIndex));
    //}
    //if (message.size() == 1 && message.getAddressPattern() == "/track/name" && message[0].isString())
    //{
    //    String trackName = message[0].getString();
    //    DBG("Last track name: " + trackName);
    //    if (trackName.startsWith("##"))
    //    {
    //        trackNameArray.set(trackArrayIndex, trackName);
    //        trackArrayIndex++;
    //    }
    //}
    
}

void StimulusPlayer::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &openButton)
    {
        browseForConfigFile();
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

void StimulusPlayer::changeListenerCallback(ChangeBroadcaster* source)
{
    logWindow.setText(logWindowMessage);
    logWindow.moveCaretToEnd();
    loadedFileName.setText("Loaded file: " + currentlyLoadedFile.getFileName(), dontSendNotification);
}

void StimulusPlayer::timerCallback()
{
    double currentPosition = transportSource.getCurrentPosition();
    double lengthInSeconds = transportSource.getLengthInSeconds();
    
    playbackHeadPosition.setText("Time: " + returnHHMMSS(currentPosition) + " / " + returnHHMMSS(lengthInSeconds), dontSendNotification);
}

void StimulusPlayer::browseForConfigFile()
{
    
#if JUCE_MODAL_LOOPS_PERMITTED
    const bool useNativeVersion = true;
    FileChooser fc("Choose a file to open...",
                   File::getCurrentWorkingDirectory(),
                   "*.csv",
                   useNativeVersion);
    
    if (fc.browseForFileToOpen())
    {
        File chosenFile = fc.getResult();
        createFilePathList(chosenFile.getFullPathName());
        numberOfStimuli = filePathList.size();
        createStimuliTriggerButtons(); // only for test
    }
#endif
}

void StimulusPlayer::createStimuliTriggerButtons()
{
    
    // StringArray triggerButtonAlphabet = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };
    for (int i = 0; i < numberOfStimuli; ++i)
    {
        triggerStimuliButtonArray.add(new TextButton());
        triggerStimuliButtonArray[i]->getProperties().set("triggerStimuliButton", true);
        triggerStimuliButtonArray[i]->getProperties().set("buttonIndex", i);
        //if (i < triggerButtonAlphabet.size()) triggerStimuliButtonArray[i]->setButtonText(triggerButtonAlphabet[i]);
        //else triggerStimuliButtonArray[i]->setButtonText(String(i+1));
        triggerStimuliButtonArray[i]->setButtonText(fileIdList[i]);
        triggerStimuliButtonArray[i]->addListener(this);
        addAndMakeVisible(triggerStimuliButtonArray[i]);
    }
    resized();
}
