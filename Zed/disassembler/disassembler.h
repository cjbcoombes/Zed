#pragma once
#include "../utils/utils.h"
#include "../utils/bytecode.h"

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
	class DisassemblerException : public std::exception {
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

		const ErrorType eType;
		std::string extra;

		DisassemblerException(const ErrorType& eType) : eType(eType), extra("") {}
		DisassemblerException(const ErrorType& eType, char* const& extra) : eType(eType), extra(extra) {}
		DisassemblerException(const ErrorType& eType, const char* const& extra) : eType(eType), extra(extra) {}
		DisassemblerException(const ErrorType& eType, const std::string& extra) : eType(eType), extra(extra) {}

		virtual const char* what();
	};
	
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Disassembler Functions

	// Disassembles from an input file to an output file
	int disassemble(const char* const& inputPath, const char* const& outputPath, DisassemblerSettings& settings);
	int disassemble_(std::iostream& inputFile, std::iostream& outputFile, DisassemblerSettings& settings, std::ostream& stream);
}