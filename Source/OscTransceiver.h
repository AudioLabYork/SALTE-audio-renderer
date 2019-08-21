#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class OscTransceiver : public OSCReceiver
{
public:
	OscTransceiver();
	~OscTransceiver();

	void connectTxRx(String ipToSendTo, int portToSendTo, int portToReceiveAt);
	void disconnectTxRx();
	bool isConnected();
	
	template <typename... Args>
	void sendOscMessage(const String& message, Args&& ... args);
private:
	
	OSCSender sender;
	void showConnectionErrorMessage(const String& messageText);
	bool isSenderConnected = false, isReceiverConnected = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscTransceiver)
};

template <typename... Args>
void OscTransceiver::sendOscMessage(const String& message, Args&& ... args)
{
	if (!sender.send(message, std::forward<Args>(args)...))
		showConnectionErrorMessage("Error: could not send OSC message.");
}