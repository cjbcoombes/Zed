#include "compiler.h"


//#define CMP_WRITERAW(thing, sz) outputFile.write(thing, sz); byteCounter += sz
//#define CMP_WRITE(thing, type) outputFile.write(TO_CH_PT(thing), sizeof(type)); byteCounter += sizeof(type)

template<typename T>
void compiler::gen::GenState::write(T obj) {
	outputFile.write(reinterpret_cast<char*>(&obj), sizeof(T));
	byteCounter += sizeof(T);
}

int compiler::generateBytecode(std::iostream& outputFile, ast::Tree& astTree, CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler;
	using namespace compiler::gen;
	using namespace bytecode;
	
	GenState state(outputFile);

	state.write<types::word_t>(0); // First instruction address (to be overwritten later)

	return 0;
}