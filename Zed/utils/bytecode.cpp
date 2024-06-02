#include "bytecode.h"
#include <iostream>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Program

bytecode::Program::Program(std::iostream& program) {
	// https://stackoverflow.com/questions/22984956/tellg-function-give-wrong-size-of-file
	program.seekg(0, std::ios::beg);
	program.ignore(std::numeric_limits<std::streamsize>::max());
	const std::streamsize length = program.gcount();

	owner = std::make_unique<char[]>(static_cast<size_t>(length) + FILLER_SIZE);
	start = owner.get();
	ip = start;
	end = start + length;

	program.clear();
	program.seekg(0, std::ios::beg);
	program.read(start, length);

	std::fill_n(start + length, FILLER_SIZE, bytecode::Opcode::HALT);
}

char* bytecode::Program::pos() const noexcept {
	return ip;
}

char* bytecode::Program::begin() const noexcept {
	return start;
}

int bytecode::Program::offset() const noexcept {
	return ip - start;
}

bool bytecode::Program::inBounds() const noexcept {
	return start <= ip && ip < end;
}

void bytecode::Program::goto_(const types::word_t loc) noexcept {
	ip = start + loc;
}

template<typename T>
void bytecode::Program::read(T* val) noexcept {
	(*val) = *reinterpret_cast<T*>(ip);
	ip += sizeof(T);
}