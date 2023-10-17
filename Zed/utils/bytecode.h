#pragma once
#include "utils.h"
#include "opcode.h"


namespace bytecode {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constants?

	constexpr int FIRST_INSTR_ADDR_LOCATION = 0;
	constexpr int GLOBAL_TABLE_LOCATION = 4;
	constexpr int NUM_WORD_REGISTERS = 14;
	constexpr int NUM_BYTE_REGISTERS = 14;

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Types (could maybe be its own file)

	// Typedefs for the specific types used by the bytecode
	namespace types {
		// 32-bit word: addresses, integers
		typedef int32_t word_t;
		// 8-bit byte: register IDs, opcode IDs, chars
		typedef int8_t byte_t;

		// Opcode ID: byte
		typedef uint8_t opcode_t;
		// Register ID: byte
		typedef uint8_t reg_t;

		// Integer: word
		typedef int32_t int_t;
		typedef float float_t;
		// Char: byte
		typedef int8_t char_t;
		// Bool: byte
		typedef int8_t bool_t;

		union WordVal {
			word_t word;

			int_t int_;
			float_t float_;
		};

		union ByteVal {
			byte_t byte;

			char_t char_;
			bool_t bool_;
		};

		static_assert(sizeof(std::intptr_t) == sizeof(word_t), "No workaround for non-word-size (32-bit) pointers");
		static_assert(sizeof(float) == sizeof(word_t), "No workaround for non-word-size (32-bit) floats");
	}

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Register data

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Register | ID		| Desc
	// -		| IP		| Instruction pointer
	// 0		| BP		| Stack base pointer (moves with the current stack frame)
	// 1		| RP		| Stack root pointer (always the start of the stack, where global variables are)
	// 2		| PP		| Program memory pointer (points to the base of the binary file loaded into memory)
	// 3		| FZ		| Zero flag (set automatically by arithmetic operations, zero if the result is zero, one otherwise)
	// 4 ... 17	| W0 .. 13	| General purpose word
	// 18 .. 31	| B0 .. 13	| General purpose byte

	namespace reg {
		enum {
			BP = 0,	// 0
			RP = 1, // 1
			PP = 2,	// 2
			FZ = 3,	// 3
			W0 = 4,	// 4
			B0 = 18,
			COUNT = 32
		};
	}

	constexpr int namedRegCount = reg::W0;
	constexpr const char* const regStrings[] = {
		"BP",
		"RP",
		"PP",
		"FZ"
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Program

	// A .eze program loaded into memory
	class Program {
		static constexpr int FILLER_SIZE = 24;

	public:
		char* start;
		char* ip;
		char* end;

		Program(std::iostream& program);
		~Program();

		void goto_(types::word_t loc);

		template<typename T>
		void read(T* val);
	};
}