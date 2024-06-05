#include "string_lookup.h"
#include "flags.h"
#include "code_location.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// String Array Utils

int lookupString(const char* const match, const std::span<const char* const>& strings) noexcept {
	int i = 0;

	for (const char* const str : strings) {
		if (!strcmp(match, str)) {
			return i;
		}
		i++;
	}

	return -1;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Code Location Struct

code_location::code_location(const int line, const int column) : startLine(line), startCol(column), endLine(line), endCol(column) {}
code_location::code_location(const int startLine, const int startCol, const int endLine, const int endCol)
	: startLine(startLine), startCol(startCol), endLine(endLine), endCol(endCol) {}

code_location::code_location(const code_location& start, const code_location& end)
	: startLine(start.startLine), startCol(start.startCol), endLine(end.endLine), endCol(end.endCol) {
}
code_location::code_location() : startLine(-1), startCol(-1), endLine(-1), endCol(-1) {}
bool code_location::isValid() const {
	return startLine >= 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Flags Struct

Flags::Flags() noexcept : bits(0) {}
Flags::Flags(const bits_t bits) noexcept : bits(bits) {}

bool Flags::hasFlags(const bits_t flags) const noexcept {
	return !(flags & (~bits));
}

bool Flags::hasFlags(const Flags& other) const noexcept {
	return hasFlags(other.bits);
}

void Flags::setFlags(const bits_t flags) noexcept  {
	bits |= flags;
}

void Flags::setFlags(const Flags& other) noexcept {
	setFlags(other.bits);
}

void Flags::unsetFlags(const bits_t flags) noexcept {
	bits &= ~flags;
}

void Flags::toggleFlags(const bits_t flags) noexcept {
	bits ^= flags;
}