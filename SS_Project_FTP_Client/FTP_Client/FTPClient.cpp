#include "FTPClient.h"
#include <iostream>
#include "utils.h"
#include "bout.h"

FTPClient::FTPClient(const char* ip_address, int port_number, std::function<void(const char*)> print_callback)
{
	this->print_callback = print_callback;
	std::function<void(const char*)> line_received_callback = std::bind(&FTPClient::on_line_received, this, std::placeholders::_1);
	telnet_client = new TelNetClient(ip_address, port_number, line_received_callback);
	is_connected = true;
	virtual_filesystem = new VirtualFS("vfs_root");
}


void FTPClient::on_line_received(const char* line)
{
	strncpy_s(line_buffer, line, sizeof(line_buffer));

	std::cout << Utils::Color::Yellow();
	print_callback(line);
	std::cout << Utils::Color::White();
}


int FTPClient::execute_command(const char* command)
{
	std::cout << Utils::Color::Blue() << command << Utils::Color::White() << "\n";
	return telnet_client->sendCommand(command);
}

void FTPClient::login(const char* username, const char* password)
{
	if (!is_connected)
	{
		telnet_client->reconnect();
		is_connected = true;
	}
	if (execute_command(bout() << "USER " << username << bfin) != 331)
	{
		throw std::exception("login failed");
	}
	if (execute_command(bout() << "PASS " << password << bfin) != 230)
	{
		throw std::exception("login failed");
	}
}

void FTPClient::logout()
{
	if (execute_command("QUIT") != 221)
	{
		throw std::exception("logout failed");
	}
	is_connected = false;
	telnet_client->closeConnection();
}

void FTPClient::list_files(const char* directory)
{
	int response = directory == nullptr ? execute_command("LIST") : execute_command(bout() << "LIST " << directory << bfin);
	if (response != 150)
		throw std::exception("Failed");

	std::vector<char> temp_buffer(1024);
	std::vector<char> file_buffer;
	int temp_size = 0;

	while ((temp_size = data_port.recv(temp_buffer.data(), temp_buffer.size()).bytes_count) > 0)
	{
		file_buffer.insert(file_buffer.end(), temp_buffer.begin(), temp_buffer.begin() + temp_size);
	}
	data_port.close();

	file_buffer.push_back('\0');

	printf(file_buffer.data());

	if (telnet_client->receiveResponse() != 226)
		throw std::exception("Failed transfer");
}

void FTPClient::set_binary_mode()
{
	if (execute_command("TYPE I") != 200)
		throw std::exception("Failed");
}

void FTPClient::set_ascii_mode()
{
	if (execute_command("TYPE A") != 200)
		throw std::exception("Failed");
}

void FTPClient::store_file(const char* filepath)
{
	std::vector<char> file_data;

	try
	{
		file_data = virtual_filesystem->readFile(filepath);
	}
	catch (const std::exception& e)
	{
		data_port.close();
		throw e;
	}

	if (execute_command(bout() << "STOR " << filepath << bfin) != 150)
		throw std::exception("Failed");

	data_port.send(file_data.data(), file_data.size());
	data_port.close();

	if (telnet_client->receiveResponse() != 226)
		throw std::exception("Failed transfer");
}

void FTPClient::retrieve_file(const char* filepath)
{
	if (execute_command(bout() << "RETR " << filepath << bfin) != 150)
		throw std::exception("Failed");

	std::vector<char> temp_buffer(1024);
	std::vector<char> file_data;
	int temp_size = 0;

	while ((temp_size = data_port.recv(temp_buffer.data(), temp_buffer.size()).bytes_count) > 0)
	{
		file_data.insert(file_data.end(), temp_buffer.begin(), temp_buffer.begin() + temp_size);
	}
	data_port.close();

	virtual_filesystem->writeFile(filepath, file_data);

	if (telnet_client->receiveResponse() != 226)
		throw std::exception("Failed transfer");
}

namespace
{
	void parse_passive_address(const char* buffer, int address[6])
	{
		constexpr int maxStrLen = 4 * 6;
		int i = 0, k = 0;

		for (; buffer[k] != ')' && k < maxStrLen; k++)
		{
			if ('0' <= buffer[k] && buffer[k] <= '9')
			{
				address[i] = address[i] * 10 + (buffer[k] - '0');
				continue;
			}
			if (buffer[k] == ',')
			{
				if (i >= 6)
					throw std::exception("Failed to parse PASV address: too many numbers");
				i++;
				continue;
			}
			throw std::exception(bout() << "Failed to parse PASV address: invalid character '0x" << bhex << buffer[i] << "'" << bfin);
		}

		if (buffer[k] != ')')
			throw std::exception("Failed to parse PASV address: input too long");
		else
		{
			if (i >= 6)
				throw std::exception("Failed to parse PASV address: too many numbers");
			i++;
		}

		if (i < 6)
			throw std::exception("Failed to parse PASV address: insufficient numbers");
	}
}

void FTPClient::enter_passive_mode()
{
	if (execute_command("PASV") != 227)
		throw std::exception("Entering passive mode failed");
	const char* line_prefix = "227 Entering Passive Mode (";
	if (strncmp(line_prefix, line_buffer, strlen(line_prefix)) != 0)
		throw std::exception("Invalid passive response message");
	char* buffer = line_buffer + strlen(line_prefix);

	int address[6]{};
	parse_passive_address(buffer, address);

	const char* ip_address = bout() << address[0] << "." << address[1] << "." << address[2] << "." << address[3] << bfin;
	int port_number = address[4] * 256 + address[5];

	data_port.connect(ip_address, port_number);
	printf("Opened data port on %s:%i.\n", (const char*)ip_address, port_number);
}

FTPClient::~FTPClient()
{
	delete telnet_client;
	delete virtual_filesystem;
}
