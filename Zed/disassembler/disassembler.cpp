#include "disassembler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Disassembler Exceptions

const char* disassembler::DisassemblerException::what() {
	if (extra.length() == 0) {
		return errorTypeStrings[static_cast<int>(eType)];
	} else {
		extra.insert(0, " : ");
		extra.insert(0, errorTypeStrings[static_cast<int>(eType)]);
		return extra.c_str();
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Disassembler Functions

int disassembler::disassemble(const char* const& inputPath, const char* const& outputPath, DisassemblerSettings& settings) {
	using namespace disassembler;
	using std::cout;

	cout << IO_MAIN "Attempting to disassemble file \"" << inputPath << "\" into output file \"" << outputPath << "\"\n" IO_NORM;

	std::fstream inputFile, outputFile;

	inputFile.open(inputPath, std::ios::in | std::ios::binary);
	if (!inputFile.is_open()) {
		cout << IO_ERR "Could not open file \"" << inputPath << "\"" IO_NORM IO_END;
		return 1;
	}

	outputFile.open(outputPath, std::ios::out | std::ios::trunc);
	if (!outputFile.is_open()) {
		cout << IO_ERR "Could not open file \"" << outputPath << "\"" IO_NORM IO_END;
		return 1;
	}

	try {
		int out = disassembler::disassemble_(inputFile, outputFile, settings, cout);
		cout << IO_MAIN "Disassembly finished with code " << out << IO_NORM IO_END;
		return out;
	} catch (DisassemblerException& e) {
		cout << IO_ERR "Error during disassembly : " << e.what() << IO_NORM IO_END;
	} catch (std::exception& e) {
		cout << IO_ERR "An unknown error ocurred during disassembly. This error is most likely an issue with the c++ disassembler code, not your code. Sorry. The provided error message is as follows:\n" << e.what() << IO_NORM IO_END;
	}

	return 1;
}

int disassembler::disassemble_(std::iostream& inputFile, std::iostream& outputFile, DisassemblerSettings& settings, std::ostream& stream) {
	using namespace bytecode::types;
	using namespace bytecode::Opcode;
	using namespace bytecode;

	Program program(inputFile);
	program.goto_(bytecode::FIRST_INSTR_ADDR_LOCATION);
	program.goto_(*reinterpret_cast<types::word_t*>(program.ip));

	opcode_t opcode = 0;
	reg_t rid1 = 0;
	word_t word = 0;
	byte_t byte = 0;

	while (program.ip < program.end) {
		program.read<opcode_t>(&opcode);
		if (opcode >= Opcode::ValidCount) {
			throw DisassemblerException(DisassemblerException::ErrorType::INVALID_OPCODE);
		}
		outputFile << std::left << std::setw(10) << opcodeStrings[opcode] << std::right << "  ";
		for (const int& arg : opcodeArgs[opcode]) {
			switch (static_cast<OpcodeArgType>(arg)) {
				case OpcodeArgType::ARG_WORD_REG:
					program.read<reg_t>(&rid1);
					if (rid1 == reg::BP || rid1 == reg::RP || rid1 == reg::PP) {
						outputFile << std::left << std::setw(10) << regStrings[rid1] << std::right << "  ";
					} else if (rid1 >= reg::W0 && rid1 < reg::B0) {
						outputFile << "W" << std::left << std::setw(9) << (rid1 - reg::W0) << std::right << "  ";
					} else {
						throw DisassemblerException(DisassemblerException::ErrorType::INVALID_WORD_REG);
					}
					break;
				case OpcodeArgType::ARG_BYTE_REG:
					program.read<reg_t>(&rid1);
					if (rid1 == reg::FZ) {
						outputFile << std::left << std::setw(10) << regStrings[rid1] << std::right << "  ";
					} else if (rid1 >= reg::B0 && rid1 < reg::COUNT) {
						outputFile << "B" << std::left << std::setw(9) << (rid1 - reg::B0) << std::right << "  ";
					} else {
						throw DisassemblerException(DisassemblerException::ErrorType::INVALID_BYTE_REG);
					}
					break;
				case OpcodeArgType::ARG_WORD:
					program.read<word_t>(&word);
					outputFile << "0x" << std::setfill('0') << std::setw(8) << IO_HEX << word << IO_DEC << std::setfill(' ') << "  ";
					break;
				case OpcodeArgType::ARG_BYTE:
					program.read<byte_t>(&byte);
					outputFile << "0x" << std::setfill('0') << std::setw(2) << IO_HEX << byte << IO_DEC << std::setfill(' ') << "      " "  ";
					break;
			}
		}
		outputFile << "\n";
	}

	return 0;
}