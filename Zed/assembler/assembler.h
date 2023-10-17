#pragma once
#include "..\utils\utils.h"
#include "..\utils\bytecode.h"

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
	class AssemblerException : public std::exception {
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

		const ErrorType eType;
		const int line;
		const int column;
		std::string extra;

		AssemblerException(const ErrorType& eType, const int& line, const int& column) : eType(eType), line(line), column(column), extra("") {}
		AssemblerException(const ErrorType& eType, const int& line, const int& column, char* const& extra) : eType(eType), line(line), column(column), extra(extra) {}
		AssemblerException(const ErrorType& eType, const int& line, const int& column, const char* const& extra) : eType(eType), line(line), column(column), extra(extra) {}
		AssemblerException(const ErrorType& eType, const int& line, const int& column, const std::string& extra) : eType(eType), line(line), column(column), extra(extra) {}

		virtual const char* what();
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Assembler Functions

	// Assembles from an input file to an output file
	int assemble(const char* const& inputPath, const char* const& outputPath, AssemblerSettings& settings);
	int assemble_(std::iostream& inputFile, std::iostream& outputFile, AssemblerSettings& settings, std::ostream& stream);

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Parsing Helper Functions

	bytecode::types::reg_t parseWordRegister(char* const& str, const int& strlen, const int& line, const int& column);
	bytecode::types::reg_t parseByteRegister(char* const& str, const int& strlen, const int& line, const int& column);
	// Parses a number to a particular type
	template<typename T, AssemblerException::ErrorType eType>
	T parseNumber(const char* str, int strlen, const int& line, const int& column);

	// Declare for number types
	template bytecode::types::reg_t parseNumber<bytecode::types::reg_t, AssemblerException::ErrorType::INVALID_WORD_REG_PARSE>(const char*, int, const int&, const int&);
	template bytecode::types::reg_t parseNumber<bytecode::types::reg_t, AssemblerException::ErrorType::INVALID_BYTE_REG_PARSE>(const char*, int, const int&, const int&);
	template bytecode::types::word_t parseNumber<bytecode::types::word_t, AssemblerException::ErrorType::INVALID_WORD_PARSE>(const char*, int, const int&, const int&);
	template bytecode::types::byte_t parseNumber<bytecode::types::byte_t, AssemblerException::ErrorType::INVALID_BYTE_PARSE>(const char*, int, const int&, const int&);
}