#pragma once

#include "TCPResult.h"
#include "TCPResponse.h"

class TCP final
{
private:
    class Impl;
    Impl* privates;

public:
    TCP();

    void connect(const char* host, int port);

    TCPResult send(const void* buffer, size_t size);
    TCPResult recv(void* buffer, size_t size);

    void ensureSend(void* buffer, size_t size);
    void ensureRecv(void* buffer, size_t size);

    TCPResult sendInt32(int n);
    TCPResponse<int> recvInt32();

    TCPResult sendUInt8(unsigned char n);
    TCPResponse<unsigned char> recvUInt8();

    TCPResult sendInt8(char n);
    TCPResponse<char> recvInt8();

    void setTimeout(int seconds);

    int getPort() const;
    const char* getIpAddress() const;

    void close();

    ~TCP();
};
