#include "utils.h"

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
// Flags Struct

Flags::Flags() noexcept : bits(0) {}
Flags::Flags(const int bits) noexcept : bits(bits) {}

bool Flags::hasFlags(const int flags) const noexcept {
	return !(flags & (~bits));
}

bool Flags::hasFlags(const Flags& other) const noexcept {
	return hasFlags(other.bits);
}

void Flags::setFlags(const int flags) noexcept  {
	bits |= flags;
}

void Flags::setFlags(const Flags& other) noexcept {
	setFlags(other.bits);
}

void Flags::unsetFlags(const int flags) noexcept {
	bits &= ~flags;
}

void Flags::toggleFlags(const int flags) noexcept {
	bits ^= flags;
}