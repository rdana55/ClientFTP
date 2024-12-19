#include "TelNetClient.h"
#include <exception>
#include <bout.h>

TelNetClient::TelNetClient(const char* ip_address, int port_number, std::function<void(char*)> line_received_callback) : line_received_callback_{ line_received_callback },
ip_address_{ ip_address }, port_number_{ port_number }
{
	try
	{
		tcp_.connect(ip_address_, port_number_);
		tcp_.setTimeout(3);

		printf("Client: %s:%i\n", tcp_.getIpAddress(), tcp_.getPort());

		receiveResponse(); // Server greeting		
	}
	catch (std::exception& e)
	{
		throw e;
	}
}

void TelNetClient::reconnect()
{
	printf("Reconnecting...\n");
	if (is_connected_)
		closeConnection();
	tcp_.connect(ip_address_, port_number_);
	receiveResponse(); // Server greeting		
	is_connected_ = true;
}

void TelNetClient::closeConnection()
{
	is_connected_ = false;
	tcp_.close();
}

int TelNetClient::sendCommand(const char* command)
{
	command = bout() << command << "\r\n" << bfin;
	size_t length = strlen(command);
	tcp_.send(command, length);
	return receiveResponse();
}


namespace
{
	void readLine(TCP& tcp, char* buffer)
	{
		char* c = buffer;
		for (; (*c = tcp.recvUInt8()) != 0x0A; c++);
		*(++c) = '\0';
	}
}

namespace
{
	int responseCodeToInt(const char* code)
	{
		return (code[0] - '0') * 100 + (code[1] - '0') * 10 + (code[2] - '0');
	}
}

int TelNetClient::receiveResponse()
{
	char first_line[2048] = { 0 };
	char buffer[2048] = { 0 };

	readLine(tcp_, first_line);
	line_received_callback_(first_line);

	if (first_line[3] == ' ') return responseCodeToInt(first_line);

	while (buffer[0] != first_line[0] || buffer[1] != first_line[1] || buffer[2] != first_line[2] || buffer[3] != ' ')
	{
		readLine(tcp_, buffer);
		line_received_callback_(buffer);
	}

	return responseCodeToInt(first_line);
}
