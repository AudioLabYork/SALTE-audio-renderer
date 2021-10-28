#include "OscTransceiver.h"

OscTransceiver::OscTransceiver()
{
}

OscTransceiver::~OscTransceiver()
{
	disconnectTxRx();
}

void OscTransceiver::connectTxRx(String ipToSendTo, int portToSendTo, int portToReceiveAt)
{
	if (!sender.connect(ipToSendTo, portToSendTo))
		sendMsgToLogWindow("Error: could not connect sender to UDP port: " + String(portToSendTo));
	else
		isSenderConnected = true;

	if (!connect(portToReceiveAt))
		sendMsgToLogWindow("Error: could not connect receiver to UDP port " + String(portToReceiveAt));
	else
		isReceiverConnected = true;
}

void OscTransceiver::disconnectTxRx()
{
	if (!sender.disconnect())
		sendMsgToLogWindow("Error: could not disconnect sender from the currently used UDP port");
	else
		isSenderConnected = false;

	if (!disconnect())
		sendMsgToLogWindow("Error: could not disconnect receiver from the currently used UDP port.");
	else
		isReceiverConnected = false;
}

bool OscTransceiver::isConnected()
{
	if (isSenderConnected && isReceiverConnected)
		return true;
	else
		return false;
}

//void OscTransceiver::showConnectionErrorMessage(const String& messageText)
//{
//	AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
//		"Connection error",
//		messageText,
//		"OK");
//}

void OscTransceiver::sendMsgToLogWindow(String message)
{
	currentMessage += message + "\n";
	sendChangeMessage();  // broadcast change message to inform and update the editor
}
