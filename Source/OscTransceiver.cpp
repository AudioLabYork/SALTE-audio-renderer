#include "OscTransceiver.h"

oscTransceiver::oscTransceiver()
{
}

oscTransceiver::~oscTransceiver()
{
}

void oscTransceiver::connectSender(String ipToSendTo, int portToSendTo)
{
	if (!sender.connect(ipToSendTo, portToSendTo))
		showConnectionErrorMessage("Error: could not connect sender to UDP port: " + String(portToSendTo));
}

void oscTransceiver::disconnectSender()
{
	if (!sender.disconnect())
		showConnectionErrorMessage("Error: could not disconnect sender from the currently used UDP port");
}

void oscTransceiver::connectReceiver(int portToReceiveAt)
{
	if (!connect(portToReceiveAt))
		showConnectionErrorMessage("Error: could not connect receiver to UDP port " + String(portToReceiveAt));
}

void oscTransceiver::disconnectReceiver()
{
	if (!disconnect())
		showConnectionErrorMessage("Error: could not disconnect receiver from the currently used UDP port.");
}

void oscTransceiver::sendOscMessage(const String& message)
{
	if (!sender.send(message))
		showConnectionErrorMessage("Error: could not send OSC message.");
}


void oscTransceiver::sendOscMessage(const String& message, int value)
{
	if (!sender.send(message, value))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void oscTransceiver::sendOscMessage(const String& message, int value, int value2)
{
	if (!sender.send(message, value, value2))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void oscTransceiver::sendOscMessage(const String& message, float value)
{
	if (!sender.send(message, value))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void oscTransceiver::sendOscMessage(const String& message, String value)
{
	if (!sender.send(message, value))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void oscTransceiver::sendOscMessage(const String& message, int value, float value2)
{
	if (!sender.send(message, value, value2))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void oscTransceiver::sendOscMessage(const String& message, float value, float value2, float value3, float value4)
{
	if (!sender.send(message, value, value2, value3, value4))
		showConnectionErrorMessage("Error: could not send OSC message.");
}

void oscTransceiver::showConnectionErrorMessage(const String& messageText)
{
	AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
		"Connection error",
		messageText,
		"OK");
}
