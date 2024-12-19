#pragma once

#include "TCP.h"
#include "TelNetClient.h"
#include <functional>
#include "VirtualFS.h"

class FTPClient
{
public:
	static constexpr int MAX_LINE_BUFF_SIZE = 2048;
private:
	bool is_connected = false;
	TelNetClient* telnet_client;
	TCP data_port;
	std::function<void(const char*)> print_callback;
	void on_line_received(const char*);
	int execute_command(const char*);
	char line_buffer[MAX_LINE_BUFF_SIZE];
	VirtualFS* virtual_filesystem;

public:
	FTPClient(const char* ip_address, int port_number = 21, std::function<void(const char*)> print_callback = [](const char*) {});

	void login(const char* username, const char* password);
	void logout();
	void list_files(const char* directory);
	void enter_passive_mode();

	void store_file(const char* filepath);
	void retrieve_file(const char* filepath);

	void set_binary_mode();
	void set_ascii_mode();

	~FTPClient();
};
