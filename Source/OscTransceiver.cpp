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
		showConnectionErrorMessage("Error: could not connect sender to UDP port: " + String(portToSendTo));
	else
		isSenderConnected = true;

	if (!connect(portToReceiveAt))
		showConnectionErrorMessage("Error: could not connect receiver to UDP port " + String(portToReceiveAt));
	else
		isReceiverConnected = true;
}

void OscTransceiver::disconnectTxRx()
{
	if (!sender.disconnect())
		showConnectionErrorMessage("Error: could not disconnect sender from the currently used UDP port");
	else
		isSenderConnected = false;

	if (!disconnect())
		showConnectionErrorMessage("Error: could not disconnect receiver from the currently used UDP port.");
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

void OscTransceiver::showConnectionErrorMessage(const String& messageText)
{
	AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
		"Connection error",
		messageText,
		"OK");
}
