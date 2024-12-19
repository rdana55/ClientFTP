#include "VirtualFS.h"

#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace
{
	fs::path getAbsolutePath(fs::path root, fs::path relative)
	{
		if (relative.string()._Starts_with("/"))
			relative = fs::path("." + relative.string());
		return root / relative;
	}
}

VirtualFS::VirtualFS(std::filesystem::path root_directory) : root_directory_{ root_directory }
{
	if (!fs::exists(root_directory_))
		fs::create_directory(root_directory_);
}

std::vector<char> VirtualFS::readFile(std::filesystem::path relative_path)
{
	fs::path absolute_path = getAbsolutePath(root_directory_, relative_path);
	std::ifstream file_stream(absolute_path, std::ios::binary);

	std::cout << "Reading path: " << absolute_path << "\n";
	if (file_stream.fail())
	{
		throw std::exception((std::string("File not found: ") + absolute_path.string()).c_str());
	}

	file_stream.seekg(0, std::ios::end);
	size_t file_size = file_stream.tellg();
	file_stream.seekg(0, std::ios::beg);
	std::cout << "File size : " << file_size << "\n";
	std::vector<char> buffer(file_size);
	file_stream.read(buffer.data(), file_size);

	if (file_stream.fail())
		throw std::exception("File reading failed");

	file_stream.close();

	return buffer;
}

void VirtualFS::writeFile(std::filesystem::path relative_path, std::vector<char> buffer)
{
	fs::path absolute_path = getAbsolutePath(root_directory_, relative_path);
	std::cout << "Writing path: " << absolute_path << "\n";
	std::ofstream file_stream(absolute_path, std::ios::binary);

	if (file_stream.fail())
	{
		throw std::exception((std::string("Unable to write file: ") + absolute_path.string()).c_str());
	}

	file_stream.write(buffer.data(), buffer.size());

	if (file_stream.fail())
		throw std::exception("File writing failed");

	file_stream.close();
}
