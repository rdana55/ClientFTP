#include "TCPResult.h"

#include <winsock.h>
#include "tcp_exception.h"
#include <bout.h>

const char* TCPResult::getErrorMessage()
{
    wchar_t message_buffer[256]{};
    message_buffer[0] = '\0';

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        message_buffer,
        sizeof(message_buffer),
        NULL);

    return bout() << "Socket error " << error_code << ": " << message_buffer << bfin;
}

void TCPResult::validateSend(size_t expected_size)
{
    if (!ok)
        throw tcp_exception(getErrorMessage());
    if (bytes_count != expected_size)
        throw tcp_exception(bout() << "Not all bytes were sent (" << bytes_count << "/" << expected_size << ")" << bfin);
}

void TCPResult::validateRecv(size_t expected_size)
{
    if (!ok)
        throw tcp_exception(getErrorMessage());
    if (bytes_count == 0)
        throw tcp_exception("Connection interrupted during receive");
    if (bytes_count != expected_size)
        throw tcp_exception(bout() << "Not all bytes were received (" << bytes_count << "/" << expected_size << ")" << bfin);
}
