#pragma once

#include "TCP.h"
#include <functional>

class TelNetClient
{
private:
	TCP tcp_;
	std::function<void(char*)> line_received_callback_ = [](char*) {};
	const char* ip_address_ = nullptr;
	int port_number_ = 21;
	bool is_connected_ = false;
public:	

	TelNetClient(const char* ip, int port, std::function<void(char*)> line_received_callback_ = [](char*) {});
	int sendCommand(const char* command);
	int receiveResponse();

	void closeConnection();

	void reconnect();

};
