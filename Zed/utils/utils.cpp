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

code_location::code_location(const int line, const int column) : line(line), column(column) {}
code_location::code_location() : line(-1), column(-1) {}

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