#pragma once
#include <span>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// String Array Utils

// Finds the index of a string in a list of strings, or -1 if not found
int lookupString(const char* const match, const std::span<const char* const>& strings) noexcept;
