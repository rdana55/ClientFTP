#pragma once

#include <string>
#include <filesystem>
#include <vector>

class VirtualFS
{
private:
	std::filesystem::path root_directory_;
public:
	VirtualFS(std::filesystem::path root_directory_);
	std::vector<char> readFile(std::filesystem::path relative_path);
	void writeFile(std::filesystem::path relative_path, std::vector<char> buffer);	
};