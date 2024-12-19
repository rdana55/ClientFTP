#include "FTPCommandInterpreter.h"

#include <functional>

#define LAMBDA(ci, ftp, fname) ((std::function<void(const Parameter*)>)std::bind(fname, ci, ftp, std::placeholders::_1))

namespace
{

	void cmd_login(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		const char* username = params[0].getStringValue();
		const char* password = params[1].getStringValue();
		ftp->login(username, password);
	}

	void cmd_logout(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		ftp->logout();
	}

	void cmd_help(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		ci->printCommands(std::cout);
	}

	void cmd_list1(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		const char* directory = params[0].getStringValue();
		ftp->enter_passive_mode();
		ftp->list_files(directory);
	}

	void cmd_list0(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		ftp->enter_passive_mode();
		ftp->list_files(nullptr);
	}

	void cmd_pasv(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		ftp->enter_passive_mode();
	}

	void cmd_put(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		const char* filepath = params[0].getStringValue();
		ftp->enter_passive_mode();
		ftp->store_file(filepath);
	}

	void cmd_retr(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		const char* filepath = params[0].getStringValue();
		ftp->enter_passive_mode();
		ftp->retrieve_file(filepath);
	}

	void cmd_binary(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		ftp->set_binary_mode();
	}

	void cmd_ascii(CommandInterpreter* ci, FTPClient* ftp, const Parameter* params)
	{
		ftp->set_ascii_mode();
	}

}

FTPCommandInterpreter::FTPCommandInterpreter(FTPClient* ftp) : ftp{ ftp }
{
	registerCommand(LAMBDA(this, ftp, cmd_login), "login", Param(0, "user", ParameterType::STRING), Param(1, "pass", ParameterType::STRING));
	registerCommand(LAMBDA(this, ftp, cmd_help), "help");
	registerCommand(LAMBDA(this, ftp, cmd_logout), "logout");
	//registerCommand(LAMBDA(this, ftp, cmd_list1), "list", Param(0, "path", ParameterType::PATH));
	registerCommand(LAMBDA(this, ftp, cmd_list0), "list");
	//registerCommand(LAMBDA(this, ftp, cmd_pasv), "pasv");
	registerCommand(LAMBDA(this, ftp, cmd_put), "put", Param(0, "path", ParameterType::PATH));
	registerCommand(LAMBDA(this, ftp, cmd_retr), "get", Param(0, "path", ParameterType::PATH));

	registerCommand(LAMBDA(this, ftp, cmd_ascii), "ascii");
	registerCommand(LAMBDA(this, ftp, cmd_binary), "binary");
}
