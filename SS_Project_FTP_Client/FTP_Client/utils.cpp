#include "utils.h"
#include "bout.h"

std::ostream& Utils::operator<<(std::ostream& output_stream, const Utils::Color& color)
{
	if (output_stream.rdbuf() == std::cout.rdbuf())
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color.code);
	return output_stream;
}

// returns the address of the first occurrence of character in buffer, otherwise throws exception
const char* Utils::findCharacter(const char* buffer, int length, char character)
{
	for (int i = 0; i < length && *buffer && *buffer != character; i++, buffer++);
	if (*buffer == character) return buffer;
	throw std::exception(bout() << "Failed to find character: '" << character << "'" << bfin);
}

int Utils::stringToInt(const char* input)
{
	constexpr int MAX_INPUT_LENGTH = 10;
	long long result = 0;
	int sign = 1;

	for (int i = 0; i < MAX_INPUT_LENGTH && *input; i++, input++)
	{
		if (i == 0 && *input == '-')
		{
			sign = -1;
			continue;
		}
		if ('0' <= *input && *input <= '9')
		{
			result = result * 10 + (*input - '0');
			continue;
		}
		throw std::exception(bout() << "Failed to parse integer: invalid character '" << *input << "'" << bfin);
	}
	if (*input)
		throw std::exception("Failed to parse integer: input length exceeded");

	result *= sign;
	if (result >= INT_MAX || result <= INT_MIN)
		throw std::exception(bout() << "Argument out of range: " << result << bfin);
	return (int)result;
}
