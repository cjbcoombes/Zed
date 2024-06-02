#pragma once
#include <cstdint>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Flags Struct

// Holds flags set as bits
struct Flags {
public:
	typedef int64_t bits_t;
private:
	bits_t bits;

public:
	static constexpr bits_t FLAG_DEBUG = 1;
	static constexpr bits_t FLAG_PROFILE = FLAG_DEBUG << 1;

	// The first non-reserved flag, that individual settings objects can use
	static constexpr bits_t FLAG_FIRST_FREE = FLAG_PROFILE << 1;


	Flags() noexcept;
	explicit Flags(const bits_t bits) noexcept;

	// Checks if flags are set
	[[nodiscard]] bool hasFlags(const bits_t flags) const noexcept;
	[[nodiscard]] bool hasFlags(const Flags& other) const noexcept;
	// Sets flags
	void setFlags(const bits_t flags) noexcept;
	void setFlags(const Flags& other) noexcept;
	// Unsets flags
	void unsetFlags(const bits_t flags) noexcept;
	// Toggles flags
	void toggleFlags(const bits_t flags) noexcept;
};