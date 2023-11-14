#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
// #define NDEBUG to disable asserts
#include <cassert>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ANSI codes (and other io utils)

#define IO_END "\n\n\n"

#define IO_NORM "\033[0m"
#define IO_BLACK "\033[30m"
#define IO_RED "\033[31m"
#define IO_GREEN "\033[32m"
#define IO_YELLOW "\033[33m"
#define IO_BLUE "\033[34m"
#define IO_MAGENTA "\033[35m"
#define IO_CYAN "\033[36m"
#define IO_WHITE "\033[37m"
#define IO_GRAY "\033[90m"

#define IO_ERR IO_RED "[ERROR] "
#define IO_WARN IO_YELLOW "[WARNING] "
#define IO_MAIN IO_GREEN "[MAIN] "
#define IO_DEBUG IO_CYAN "[DEBUG] "

#define IO_HEX std::hex
#define IO_DEC std::dec

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// String Array Utils

// Finds the index of a string in a list of strings, or -1 if not found
int lookupString(const char* const& match, const char* const stringArr[], const int& arrLen);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Flags Struct

// Holds flags set as bits
struct Flags {
	static constexpr int FLAG_DEBUG = 1;
	static constexpr int FLAG_PROFILE = 2;

	int bits;

	Flags();
	Flags(const int& bitsIn);

	// Checks if flags are set
	bool hasFlags(const int& flags) const;
	// Sets flags
	void setFlags(const int& flags);
	// Unsets flags
	void unsetFlags(const int& flags);
	// Toggles flags
	void toggleFlags(const int& flags);
};