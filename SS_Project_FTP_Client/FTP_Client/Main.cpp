#include <iostream>
#include <exception>

#include "FTPClient.h"
#include "FTPCommandInterpreter.h"

#include <string>
#include "tcp_exception.h"
#include "utils.h"
#include "ArgsParser.h"

using namespace std;

void start_client(const char* ip_address, int port_number)
{
    FTPClient ftp_client(ip_address, port_number, printf);

    FTPCommandInterpreter command_interpreter(&ftp_client);

    std::string command;
    while (1)
    {
        std::cout << ">> ";
        std::getline(cin, command);

        try
        {
            command_interpreter.executeCommand(command.c_str());
        }
        catch (exception& e)
        {
            cout << Utils::Color::Red() << e.what() << Utils::Color::White() << "\n";
        }
    }

    return;
}

int main(int argc, const char** argv)
{
    try
    {
        ArgsParser args(argc, argv);
        const char* ip_address = args.get_arg<const char*>(1, "192.168.56.1"); //192.168.56.1 xlight   172.30.250.255 filip
        int port_number = args.get_arg(2, 2121); //2121 xlight&filip

        if (Utils::get_str_bound(ip_address, 20) < 0)
            throw std::exception("Invalid IP");

        start_client(ip_address, port_number);
    }
    catch (exception& e)
    {
        cout << e.what() << "\n";
    }
}
