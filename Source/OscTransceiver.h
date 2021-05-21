#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class OscTransceiver : public OSCReceiver
					 , public ChangeBroadcaster
{
public:
	OscTransceiver();
	~OscTransceiver();

	void connectTxRx(String ipToSendTo, int portToSendTo, int portToReceiveAt);
	void disconnectTxRx();
	bool isConnected();
	
	template <typename... Args>
	void sendOscMessage(const String& message, Args&& ... args);

	// log window message
	String currentMessage;
private:
	
	OSCSender sender;
	//void showConnectionErrorMessage(const String& messageText);
	void sendMsgToLogWindow(String message);
	bool isSenderConnected = false, isReceiverConnected = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscTransceiver)
};

template <typename... Args>
void OscTransceiver::sendOscMessage(const String& message, Args&& ... args)
{
	if (!isSenderConnected)
		sendMsgToLogWindow("Error: OSC sender not connected.");
	else if(!sender.send(message, std::forward<Args>(args)...))
		sendMsgToLogWindow("Error: could not send OSC message.");
}