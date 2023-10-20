#include "bytecode.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Program

bytecode::Program::Program(std::iostream& program) {
	// https://stackoverflow.com/questions/22984956/tellg-function-give-wrong-size-of-file
	program.seekg(0, std::ios::beg);
	program.ignore(std::numeric_limits<std::streamsize>::max());
	std::streamsize length = program.gcount();

#pragma warning (suppress : 4244) // Conversion from streamsize to unsigned int
	start = new char[length + FILLER_SIZE];
	ip = start;
	end = start + length;

	program.clear();
	program.seekg(0, std::ios::beg);
	program.read(start, length);

	std::fill(start + length, start + length + FILLER_SIZE, bytecode::Opcode::HALT);
}

bytecode::Program::~Program() {
	delete[] start;
}

void bytecode::Program::goto_(types::word_t loc) {
	ip = start + loc;
}

template<typename T>
void bytecode::Program::read(T* val) {
	(*val) = *reinterpret_cast<T*>(ip);
	ip += sizeof(T);
}