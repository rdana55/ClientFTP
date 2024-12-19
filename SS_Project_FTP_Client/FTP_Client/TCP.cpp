#include "TCP.h"

#define _WIN32_WINNT 0x601 // Windows 7 or later
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2spi.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <string>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include "bufferf.h"
#include "tcp_exception.h"
#include <bout.h>

class TCP::Impl
{
private:
	SOCKET socket_ = INVALID_SOCKET;
	addrinfo* server_info_ = nullptr;
	int port_ = 0;
	char ip_address_[100] = {};
public:

	Impl()
	{
		WSADATA wsa_data;
		int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
		if (result != 0)
		{
			throw std::exception(bout() << "WSAStartup failed with error:" << result << bfin);
		}
	}

	void setTimeout(int seconds)
	{
		DWORD timeout = seconds * 1000;
		setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
		setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	}

	void connectToServer(const char* host, int port)
	{
		closeConnection();
		socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		char port_str[20] = { 0 };
		if (_itoa_s(port, port_str, 10) != 0)
		{
			throw std::exception(bout() << "Failed to convert port number to string: " << port << bfin);
		}
		addrinfo* result = nullptr;

		int iResult = getaddrinfo(host, port_str, &hints, &result);
		if (iResult != 0)
		{
			WSACleanup();
			throw std::exception(bout() << "getaddrinfo failed with error:" << iResult << bfin);
		}

		// Attempt to connect to an address until one succeeds
		for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			printf("Trying..\n");
			// Create a SOCKET for connecting to server
			socket_ = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if (socket_ == INVALID_SOCKET) {
				WSACleanup();
				throw std::exception("socket failed with error: %ld\n", WSAGetLastError());
			}
			// Connect to server.
			int iResult = ::connect(socket_, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(socket_);
				socket_ = INVALID_SOCKET;
				continue;
			}
			server_info_ = ptr;

			sockaddr_in addr_struct = {};
			int addr_struct_len = sizeof(addr_struct);
			if (getsockname(socket_, (sockaddr*)&addr_struct, &addr_struct_len))
			{
				printf("getname failed\n");
			}
			else
			{
				strncpy_s(ip_address_, inet_ntoa(((sockaddr_in*)&addr_struct)->sin_addr), sizeof(ip_address_));
				port_ = ntohs(((sockaddr_in*)&addr_struct)->sin_port);
			}

			return;
		}

		throw std::exception("Connection failed");
	}

	int getPort() const { return port_; }
	const char* getIpAddress() const { return ip_address_; }

	int sendData(const char* buffer, size_t size)
	{
		return ::send(socket_, buffer, (int)size, 0);
	}

	int receiveData(char* buffer, size_t size)
	{
		return ::recv(socket_, buffer, (int)size, 0);
	}

	void closeConnection()
	{
		if (socket_ != INVALID_SOCKET)
		{
			closesocket(socket_);
			socket_ = INVALID_SOCKET;
		}
	}

	~Impl()
	{
		closeConnection();
	}
};


TCP::TCP()
{
	privates = new Impl();
}

void TCP::connect(const char* host, int port)
{
	privates->connectToServer(host, port);
}

TCPResult TCP::send(const void* buffer, size_t size)
{
	int sent_result = privates->sendData((const char*)buffer, size);

	if (sent_result < 0)
		return TCPResult::fail(WSAGetLastError());
	else
		return TCPResult::success(sent_result);
}

TCPResult TCP::recv(void* buffer, size_t size)
{
	int recv_result = privates->receiveData((char*)buffer, size);

	if (recv_result < 0)
		return TCPResult::fail(WSAGetLastError());
	else
		return TCPResult::success(recv_result);
}

void TCP::ensureSend(void* buffer, size_t size)
{
	send(buffer, size).validateSend(size);
}

void TCP::ensureRecv(void* buffer, size_t size)
{
	recv(buffer, size).validateRecv(size);
}

TCPResult TCP::sendInt32(int n)
{
	int x = htonl(n);
	return send(&x, sizeof(int));
}

TCPResponse<int> TCP::recvInt32()
{
	int x;
	ensureRecv(&x, sizeof(int));
	return TCPResponse<int>::success(ntohl(x));
}

TCPResult TCP::sendUInt8(unsigned char n)
{
	return send(&n, sizeof(unsigned char));
}

TCPResponse<unsigned char> TCP::recvUInt8()
{
	unsigned char x;
	ensureRecv(&x, sizeof(unsigned char));
	return TCPResponse<unsigned char>::success(x);
}

TCPResult TCP::sendInt8(char n)
{
	return send(&n, sizeof(char));
}

TCPResponse<char> TCP::recvInt8()
{
	char x;
	ensureRecv(&x, sizeof(char));
	return TCPResponse<char>::success(x);
}

void TCP::setTimeout(int seconds) { privates->setTimeout(seconds); }

int TCP::getPort() const { return privates->getPort(); }
const char* TCP::getIpAddress() const { return privates->getIpAddress(); }

void TCP::close()
{
	privates->closeConnection();
}

TCP::~TCP()
{
	delete privates;
}
