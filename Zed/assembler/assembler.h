#pragma once
#include "../utils/utils.h"
#include "../utils/bytecode.h"

namespace assembler {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constants?

	// Max string size when reading an assembly file
	constexpr int MAX_STR_SIZE = 256;

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Assembler Settings

	// Holds settings info about the assembly process
	struct AssemblerSettings {
		Flags flags;
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Assembler Exceptions

	// An error raised by the assembler
	class AssemblerException : public std::runtime_error {
	public:
		enum class ErrorType {
			STRING_TOO_LONG,
			INVALID_OPCODE_PARSE,
			INVALID_WORD_REG_PARSE,
			INVALID_BYTE_REG_PARSE,
			INVALID_WORD_PARSE,
			INVALID_BYTE_PARSE,
			INVALID_SHORT_PARSE,
			INVALID_VAR_PARSE,
			UNDEFINED_LABEL,
			UNDEFINED_VAR,
			MISPLACED_GLOBAL,
			Count
		};

		static constexpr int errorTypeCount = static_cast<int>(ErrorType::Count);
		static constexpr const char* const errorTypeStrings[] = {
			"String too long",
			"Invalid opcode during parsing",
			"Invalid word register during parsing",
			"Invalid byte register during parsing",
			"Invalid word during parsing",
			"Invalid byte during parsing",
			"Invalid short during parsing",
			"Invalid global variable during parsing",
			"Undefined label",
			"Undefined global variable",
			"Cannot attempt to set global variable after normal program opcodes"
		};

		ErrorType eType;
		int line;
		int column;

		AssemblerException(const ErrorType eType, const int line, const int column);
		AssemblerException(const ErrorType eType, const int line, const int column, const char* extra);
		AssemblerException(const ErrorType eType, const int line, const int column, const std::string& extra);
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Assembler Functions

	// Assembles from an input file to an output file
	int assemble(const char* const inputPath, const char* const outputPath, const AssemblerSettings& settings);
	int assemble_(std::iostream& inputFile, std::iostream& outputFile, const AssemblerSettings& settings, std::ostream& stream);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Parsing Helper Functions

	bytecode::types::reg_t parseWordReg(const char* const str, const int strlen, const int line, const int column);
	bytecode::types::reg_t parseByteReg(const char* const str, const int strlen, const int line, const int column);
	bytecode::types::byte_t parseByte(const char* str, int strlen, const int line, const int column);
	bytecode::types::word_t parseWord(const char* str, int strlen, const int line, const int column);
	bytecode::types::reg_t parseReg(const char* str, int strlen, const int line, const int column, const AssemblerException::ErrorType eType);
}