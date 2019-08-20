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

	//    template <typename Arg1, typename... Args>
	//    void sendOscMessage(const String& message, Arg1&& arg1, Args&&... args);

	void sendOscMessage(const String& message);
    void sendOscMessage(const String& message, int value);
	void sendOscMessage(const String& message, int value, int value2);
	void sendOscMessage(const String& message, float value);
	void sendOscMessage(const String& message, String value);
	void sendOscMessage(const String& message, int value, float value2);
	void sendOscMessage(const String& message, float value, float value2, float value3, float value4);
private:
	
	OSCSender sender;
	void showConnectionErrorMessage(const String& messageText);
	bool isSenderConnected = false, isReceiverConnected = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscTransceiver)
};
