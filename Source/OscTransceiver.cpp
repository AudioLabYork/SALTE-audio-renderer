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

void OscTransceiver::sendOscMessage(const String& message)
{
	if (!sender.send(message))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void OscTransceiver::sendOscMessage(const String& message, int value)
{
	if (!sender.send(message, value))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void OscTransceiver::sendOscMessage(const String& message, int value, int value2)
{
	if (!sender.send(message, value, value2))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void OscTransceiver::sendOscMessage(const String& message, float value)
{
	if (!sender.send(message, value))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void OscTransceiver::sendOscMessage(const String& message, String value)
{
	if (!sender.send(message, value))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void OscTransceiver::sendOscMessage(const String& message, int value, float value2)
{
	if (!sender.send(message, value, value2))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void OscTransceiver::sendOscMessage(const String& message, float value, float value2, float value3, float value4)
{
	if (!sender.send(message, value, value2, value3, value4))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void OscTransceiver::showConnectionErrorMessage(const String& messageText)
{
	AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
		"Connection error",
		messageText,
		"OK");
}
