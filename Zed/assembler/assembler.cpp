#include "assembler.h"
#include "../utils/bytecode.h"
#include "../utils/io_utils.h"
#include "../utils/string_lookup.h"
#include <fstream>
#include <optional>
#include <vector>
#include <unordered_map>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Assembler Exceptions

assembler::AssemblerException::AssemblerException(const ErrorType eType, const int line, const int column)
	: std::runtime_error(errorTypeStrings[static_cast<int>(eType)]), eType(eType), line(line), column(column) {}

assembler::AssemblerException::AssemblerException(const ErrorType eType, const int line, const int column, const char* extra)
	: std::runtime_error(std::string(errorTypeStrings[static_cast<int>(eType)]) + " : " + extra), eType(eType), line(line), column(column) {}

assembler::AssemblerException::AssemblerException(const ErrorType eType, const int line, const int column, const std::string& extra)
	: std::runtime_error(std::string(errorTypeStrings[static_cast<int>(eType)]) + " : " + extra), eType(eType), line(line), column(column) {}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Parsing Helper Functions

static bytecode::types::reg_t parseReg(const char* str, int strlen, const int line, const int column, const assembler::AssemblerException::ErrorType eType) {
	using namespace assembler;
	using bytecode::types::reg_t;

	AssemblerException ex(eType, line, column);

	reg_t base = 10;

	if (str[0] == '-') {
		throw ex;
	}

	if (strlen > 2 && str[0] == '0') {
		char t = str[1];
		if ((t < '0' || t > '9') && t != '.') {
			str += 2;
			strlen -= 2;
			switch (t) {
				case 'x':
					base = 16;
					break;

				case 'b':
					base = 2;
					break;

				case 'd':break;

				default:
					throw ex;
			}
		}
	}

	reg_t out = 0;
	char c;
	while (strlen > 0) {
		c = *str;

		if (c == '.') {
			throw ex;
		}

		out *= base;

		if ('0' <= c && c <= '9') {
			if (c - '0' >= base) throw ex;
			out += c - '0';
		} else if ('A' <= c && c <= 'Z') {
			if (c - 'A' + 10 >= base) throw ex;
			out += c - 'A' + 10;
		} else if ('a' <= c && c <= 'z') {
			if (c - 'a' + 10 >= base) throw ex;
			out += c - 'a' + 10;
		} else {
			throw ex;
		}

		strlen--;
		str++;
	}

	return out;
}

static bytecode::types::reg_t parseWordReg(const char* const str, const int strlen, const int line, const int column) {
	using namespace assembler;
	using namespace bytecode;

	AssemblerException ex(AssemblerException::ErrorType::INVALID_WORD_REG_PARSE, line, column);
	if (strlen < 2) throw ex;

	int match = lookupString(str, regSpan);

	if (match >= 0) {
		if (match == reg::PP || match == reg::BP || match == reg::RP) return match;
		else throw ex;
	} else if (str[0] == 'W') {
		const types::reg_t out = parseReg(str + 1, strlen - 1, line, column, AssemblerException::ErrorType::INVALID_WORD_REG_PARSE);

		if (out >= NUM_WORD_REGISTERS || out < 0) {
			throw ex;
		}

		return out + reg::W0;
	}

	throw ex;
}

static bytecode::types::reg_t parseByteReg(const char* const str, const int strlen, const int line, const int column) {
	using namespace assembler;
	using namespace bytecode;

	AssemblerException ex(AssemblerException::ErrorType::INVALID_BYTE_REG_PARSE, line, column);
	if (strlen < 2) throw ex;

	int match = lookupString(str, regSpan);

	if (match >= 0) {
		if (match == reg::FZ) return match;
		else throw ex;
	} else if (str[0] == 'B') {
		const types::reg_t out = parseReg(str + 1, strlen - 1, line, column, AssemblerException::ErrorType::INVALID_BYTE_REG_PARSE);

		if (out >= NUM_WORD_REGISTERS || out < 0) {
			throw ex;
		}

		return out + reg::B0;
	}

	throw ex;
}

static bytecode::types::word_t parseWord(const char* str, int strlen, const int line, const int column) {
	using namespace assembler;
	using bytecode::types::word_t;

	AssemblerException ex(AssemblerException::ErrorType::INVALID_WORD_PARSE, line, column);

	word_t base = 10;
	word_t mul = 1;

	if (str[0] == '-') {
		str++;
		strlen--;
		mul = -1;
	}

	if (strlen > 2 && str[0] == '0') {
		char t = str[1];
		if ((t < '0' || t > '9') && t != '.') {
			str += 2;
			strlen -= 2;
			switch (t) {
				case 'x':
					base = 16;
					break;

				case 'b':
					base = 2;
					break;

				case 'd':break;

				default:
					throw ex;
			}
		}
	}

	word_t out = 0;
	bool isFloat = false;
	char c;
	while (strlen > 0) {
		c = *str;

		if (c == '.') {
			isFloat = true;
			strlen--;
			str++;
			break;
		}

		out *= base;

		if ('0' <= c && c <= '9') {
			if (c - '0' >= base) throw ex;
			out += c - '0';
		} else if ('A' <= c && c <= 'Z') {
			if (c - 'A' + 10 >= base) throw ex;
			out += c - 'A' + 10;
		} else if ('a' <= c && c <= 'z') {
			if (c - 'a' + 10 >= base) throw ex;
			out += c - 'a' + 10;
		} else {
			throw ex;
		}

		strlen--;
		str++;
	}


	float floatOut = 0;
	const float floatBase = 1 / static_cast<float>(base);
	float place = floatBase;

	if (isFloat) {
		while (strlen > 0) {
			c = *str;

			if ('0' <= c && c <= '9') {
				if (c - '0' >= base) throw ex;
				floatOut += (c - '0') * place;
			} else if ('A' <= c && c <= 'Z') {
				if (c - 'A' + 10 >= base) throw ex;
				floatOut += (c - 'A' + 10) * place;
			} else if ('a' <= c && c <= 'z') {
				if (c - 'a' + 10 >= base) throw ex;
				floatOut += (c - 'a' + 10) * place;
			} else {
				throw ex;
			}

			place *= floatBase;

			strlen--;
			str++;
		}

		floatOut += static_cast<float>(out);
		floatOut *= mul;
		return *reinterpret_cast<word_t*>(&floatOut);
	}

	return out * mul;
}

static bytecode::types::byte_t parseByte(const char* str, int strlen, const int line, const int column) {
	using namespace assembler;
	using bytecode::types::byte_t;

	AssemblerException ex(AssemblerException::ErrorType::INVALID_BYTE_PARSE, line, column);

	byte_t base = 10;
	byte_t mul = 1;

	if (str[0] == '-') {
		str++;
		strlen--;
		mul = -1;
	}

	if (strlen > 2 && str[0] == '0') {
		char t = str[1];
		if ((t < '0' || t > '9') && t != '.') {
			str += 2;
			strlen -= 2;
			switch (t) {
				case 'x':
					base = 16;
					break;

				case 'b':
					base = 2;
					break;

				case 'd':break;

				default:
					throw ex;
			}
		}
	}

	byte_t out = 0;
	char c;
	while (strlen > 0) {
		c = *str;

		if (c == '.') {
			throw ex;
		}

		out *= base;

		if ('0' <= c && c <= '9') {
			if (c - '0' >= base) throw ex;
			out += c - '0';
		} else if ('A' <= c && c <= 'Z') {
			if (c - 'A' + 10 >= base) throw ex;
			out += c - 'A' + 10;
		} else if ('a' <= c && c <= 'z') {
			if (c - 'a' + 10 >= base) throw ex;
			out += c - 'a' + 10;
		} else {
			throw ex;
		}

		strlen--;
		str++;
	}

	return out * mul;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Assembler Functions

int assembler::assemble(const char* const inputPath, const char* const outputPath, const AssemblerSettings& settings) {
	using namespace assembler;
	using std::cout;

	cout << IO_MAIN "Attempting to assemble file \"" << inputPath << "\" into output file \"" << outputPath << "\"\n" IO_NORM;

	std::fstream inputFile, outputFile;

	inputFile.open(inputPath, std::ios::in);
	if (!inputFile.is_open()) {
		cout << IO_ERR "Could not open file \"" << inputPath << "\"" IO_NORM IO_END;
		return 1;
	}

	outputFile.open(outputPath, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!outputFile.is_open()) {
		cout << IO_ERR "Could not open file \"" << outputPath << "\"" IO_NORM IO_END;
		return 1;
	}

	try {
		int out = assembler::assemble_(inputFile, outputFile, settings, cout);
		cout << IO_MAIN "Assembly finished with code " << out << IO_NORM IO_END;
		return out;
	} catch (AssemblerException& e) {
		cout << IO_ERR "Error during assembly at LINE " << e.line << ", COLUMN " << e.column << " : " << e.what() << IO_NORM IO_END;
	} catch (std::exception& e) {
		cout << IO_ERR "An unknown error occurred during assembly. This error is most likely an issue with the c++ assembler code, not your code. Sorry. The provided error message is as follows:\n" << e.what() << IO_NORM IO_END;
	}

	return 1;
}

#define TO_CH_PT(thing) reinterpret_cast<char*>(&(thing))
#define ASM_DEBUG(thing) if (isDebug) stream << IO_DEBUG << thing << IO_NORM "\n"
#define ASM_WRITERAW(thing, sz) outputFile.write(thing, sz); byteCounter += sz
#define ASM_WRITE(thing, type) outputFile.write(TO_CH_PT(thing), sizeof(type)); byteCounter += sizeof(type)

int assembler::assemble_(std::iostream& inputFile, std::iostream& outputFile, const AssemblerSettings& settings, std::ostream& stream) {

	using namespace assembler;
	using namespace bytecode;
	using namespace bytecode::types;
	using namespace bytecode::Opcode;

	// Flags/settings
	const bool isDebug = settings.flags.hasFlags(Flags::FLAG_DEBUG);

	// File setup
	inputFile.clear();
	inputFile.seekg(0, std::ios::beg);
	outputFile.clear();
	outputFile.seekp(0, std::ios::beg);
	const std::streampos inputFileBeg = inputFile.tellg();
	const std::streampos outputFileBeg = outputFile.tellp();
	int byteCounter = 0;

	// String
	char str[MAX_STR_SIZE];
	int strlen = 0;
	char c = -1;
	int line = 0;
	int column = -1;

	// Booleans
	bool end = false;
	bool isComment = false;
	bool isParen = false;
	bool isStr = false;
	bool isEscaped = false;
	bool isSettingGlobals = true;

	// Labels and Vars
	struct Label {
		struct Ref {
			std::streampos pos;
			int line;
			int column;

			Ref(const std::streampos& pos, const int line, const int column) : pos(pos), line(line), column(column) {}
		};
		std::optional<word_t> val;
		std::vector<Ref> refs;

		Label() : val(std::nullopt) {}
		explicit Label(const word_t val) : val(val) {}
	};
	std::unordered_map<std::string, Label> labels;
	std::string startstr = "@__START__";
	labels[startstr].val = bytecode::GLOBAL_TABLE_LOCATION; // This gets set later, to point after global data
	labels[startstr].refs.emplace_back(outputFileBeg + static_cast<std::streamoff>(bytecode::FIRST_INSTR_ADDR_LOCATION), -1, -1);
	word_t labelPlaceholder = 0xbcbcbcbci32;
	word_t labelErr = 0xececececi32;

	// Lonely stdstr
	std::string stdstr;

	// Arguments, opcodes
	int carg = 0;
	opcode_t opcode = NOP;

	// Dummy values
	reg_t reg = 0;
	word_t word = 0;
	byte_t byte = 0;

	// Pre-assembly file writing
	word_t dummy = labelErr;
	ASM_WRITE(dummy, word_t); // First instruction addr (to be overwritten later)

	// Da big loop
	while (!end) {
		// Update char and file pos
		if (c == '\n') {
			line++;
			column = 0;
		} else {
			column++;
		}
		inputFile.get(c);
		if (inputFile.eof()) end = true;

		// Dealing with strings and comments
		if (isStr) {
			if (isEscaped) {
				if (c == 'n') {
					c = '\n';
					line--;
				} else if (c == 'c') {
					c = '\033';
				} else if (c == '0') {
					c = '\0';
				}
				// nothing on c == '"'

				isEscaped = false;
			} else {
				if (c == '"') {
					isStr = false;
					continue;
				}

				if (c == '\\') {
					isEscaped = true;
					continue;
				}
			}

			if (strlen >= MAX_STR_SIZE) {
				throw AssemblerException(AssemblerException::ErrorType::STRING_TOO_LONG, line, column);
			}
			str[strlen++] = c;
			continue;
		} else {
			if (c == '"') {
				isStr = true;
				isEscaped = false;
				continue;
			}

			if (isComment) {
				if (c == '\n') isComment = false;
				continue;
			}

			if (isParen) {
				if (c == ')') isParen = false;
				continue;
			}

			if (c == ';') {
				isComment = true;
				continue;
			}

			if (c == '(') {
				isParen = true;
				continue;
			}
		}

		// At the end of a token:
		if (c == ' ' || c == ',' || c == '\n' || c == '\t' || end) {
			if (strlen == 0) continue;
			str[strlen] = '\0';
			ASM_DEBUG((opcode == NOP ? ":: " : "|  ") << str);

			// ~~~~~~~~~~~~~~~~~~~~~~~~~~~

			if (opcode == NOP) {
				if (str[0] == '@') {
					stdstr = str;
					labels[stdstr].val = byteCounter;
					ASM_DEBUG("Label Location: " << byteCounter);
				} else {
					const int v = lookupString(str, opcodeSpan);
					if (v < 0 || v > 255) { // opcode == 255, meaning stringMatchAt returned -1
						throw AssemblerException(AssemblerException::ErrorType::INVALID_OPCODE_PARSE, line, column);
					}

					opcode = v;

					if (isSettingGlobals && opcode < Opcode::FirstGlobal) {
						isSettingGlobals = false;
						labels[startstr].val = byteCounter;
					}

					if (!isSettingGlobals) {
						if (opcode >= Opcode::FirstGlobal) {
							throw AssemblerException(AssemblerException::ErrorType::MISPLACED_GLOBAL, line, column);
						} else {
							ASM_WRITE(opcode, opcode_t);
						}
					}

					carg = 0;
					ASM_DEBUG("Opcode: " << static_cast<int>(opcode));
					if (opcodeArgs[opcode][0] == 0) opcode = NOP;
				}

			} else {
				switch (opcodeArgs[opcode][carg]) {
					case 0: // ARG_NONE
						// THIS SHOULD NEVER BE CALLED
						carg = OPCODE_MAX_ARGS;
						break;

					case 1: // ARG_WORD_REG
						reg = parseWordReg(str, strlen, line, column);
						ASM_WRITE(reg, reg_t);
						break;

					case 2: // ARG_BYTE_REG
						reg = parseByteReg(str, strlen, line, column);
						ASM_WRITE(reg, reg_t);
						break;

					case 3: // ARG_WORD
						if (str[0] == '@' || str[0] == '%') {
							stdstr = str;
							labels[stdstr].refs.emplace_back(outputFile.tellp(), line, column);
							ASM_WRITE(labelPlaceholder, word_t);
						} else {
							word = parseWord(str, strlen, line, column);
							ASM_WRITE(word, word_t);
						}
						break;

					case 4: // ARG_BYTE
						byte = parseByte(str, strlen, line, column);
						ASM_WRITE(byte, byte_t);
						break;

					case 5: // ARG_VAR
						if (str[0] == '%') {
							stdstr = str;
							labels[stdstr].val = byteCounter;
							ASM_DEBUG("Var Location: " << byteCounter);
						} else {
							throw AssemblerException(AssemblerException::ErrorType::INVALID_VAR_PARSE, line, column);
						}
						break;

					case 6: // ARG_STR
						// Basically this: ASM_WRITERAW(str, strlen + 1);
						outputFile.write(str, static_cast<std::streamsize>(strlen) + 1);
						byteCounter += strlen + 1;
						break;
				}

				carg++;
				if (carg >= OPCODE_MAX_ARGS || opcodeArgs[opcode][carg] == 0) {
					opcode = NOP;
				}
			}

			// ~~~~~~~~~~~~~~~~~~~~~~~~~~~

			strlen = 0;
			continue;
		}

		// Add char to string
		if (strlen >= MAX_STR_SIZE) {
			throw AssemblerException(AssemblerException::ErrorType::STRING_TOO_LONG, line, column);
		}
		str[strlen++] = c;
	}

	/*
	opcode = HALT;
	ASM_WRITE(opcode, opcode_t);
	*/

	for (const auto& [labelname, label] : labels) {
		if (label.val.has_value()) {
			for (const Label::Ref& ref : label.refs) {
				outputFile.seekp(ref.pos);
				outputFile.write(reinterpret_cast<char*>(const_cast<word_t*>(&label.val.value())), sizeof(word_t));
			}
		} else {
			if (labelname[0] == '@') {
				throw AssemblerException(AssemblerException::ErrorType::UNDEFINED_LABEL, label.refs[0].line, label.refs[0].column, labelname);
			} else {// pair.first[0] == '%'
				throw AssemblerException(AssemblerException::ErrorType::UNDEFINED_VAR, label.refs[0].line, label.refs[0].column, labelname);
			}
		}
	}

	ASM_DEBUG(IO_END);

	return 0;
}