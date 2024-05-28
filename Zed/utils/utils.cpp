#include "utils.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// String Array Utils

int lookupString(const char* const& match, const char* const stringArr[], const int& arrLen) {
	int i = 0;
	int j = 0;

	for (; i < arrLen; i++) {
		j = 0;
		while (true) {
			if (stringArr[i][j] != match[j]) break;
			if (match[j] == '\0') return i;
			j++;
		}
	}

	return -1;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Flags Struct

Flags::Flags() : bits(0) {}
Flags::Flags(const int& bits) : bits(bits) {}

bool Flags::hasFlags(const int& flags) const {
	return !(flags & (~bits));
}

void Flags::setFlags(const int& flags) {
	bits |= flags;
}

void Flags::unsetFlags(const int& flags) {
	bits &= ~flags;
}

void Flags::toggleFlags(const int& flags) {
	bits ^= flags;
}