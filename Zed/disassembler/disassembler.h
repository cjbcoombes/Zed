#pragma once
#include "../utils/flags.h"
#include "../utils/bytecode.h"
#include <stdexcept>

namespace disassembler {
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constants?

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Disassembler Settings

	// Holds settings info about the disassembly process
	struct DisassemblerSettings {
		Flags flags;
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Disassembler Exceptions

	// An error raised by the disassembler
	class DisassemblerException : public std::runtime_error {
	public:
		enum class ErrorType {
			INVALID_OPCODE,
			INVALID_WORD_REG,
			INVALID_BYTE_REG,
			Count
		};

		static constexpr int errorTypeCount = static_cast<int>(ErrorType::Count);
		static constexpr const char* const errorTypeStrings[] = {
			"Invalid opcode",
			"Invalid word register",
			"Invalid byte register"
		};

		ErrorType eType;

		explicit DisassemblerException(const ErrorType eType);
		DisassemblerException(const ErrorType eType, const char* const extra);
		DisassemblerException(const ErrorType eType, const std::string& extra);
	};

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Disassembler Functions

	// Disassembles from an input file to an output file
	int disassemble(const char* const inputPath, const char* const outputPath, const DisassemblerSettings& settings);
	int disassemble_(std::iostream& inputFile, std::iostream& outputFile, const DisassemblerSettings& settings, std::ostream& stream);
}