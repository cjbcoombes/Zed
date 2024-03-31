#include "compiler.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Stack Frame Management

compiler::gen::Frame::Frame() : wordRegs(), byteRegs() {
	for (int i = 0; i < bytecode::NUM_WORD_REGISTERS; i++) {
		wordRegs[i] = false;
	}
	for (int i = 0; i < bytecode::NUM_BYTE_REGISTERS; i++) {
		byteRegs[i] = false;
	}
}

bytecode::types::reg_t compiler::gen::Frame::allocw() {
	if (nextFreeWord >= bytecode::NUM_WORD_REGISTERS) {
		nextFreeWord = 0;
		while (nextFreeWord < bytecode::NUM_WORD_REGISTERS && wordRegs[nextFreeWord]) nextFreeWord++;
	}
	if (nextFreeWord >= bytecode::NUM_WORD_REGISTERS) {
		throw CompilerException(CompilerException::ErrorType::OUT_OF_REGISTERS, -1, -1);
	}

	bytecode::types::reg_t ret = bytecode::reg::W0 + nextFreeWord;
	wordRegs[nextFreeWord] = true;
	while (nextFreeWord < bytecode::NUM_WORD_REGISTERS && wordRegs[nextFreeWord]) nextFreeWord++;

	return ret;
}

bytecode::types::reg_t compiler::gen::Frame::allocb() {
	if (nextFreeByte >= bytecode::NUM_BYTE_REGISTERS) {
		nextFreeByte = 0;
		while (nextFreeByte < bytecode::NUM_BYTE_REGISTERS && byteRegs[nextFreeByte]) nextFreeByte++;
	}
	if (nextFreeByte >= bytecode::NUM_BYTE_REGISTERS) {
		throw CompilerException(CompilerException::ErrorType::OUT_OF_REGISTERS, -1, -1);
	}

	bytecode::types::reg_t ret = bytecode::reg::B0 + nextFreeByte;
	byteRegs[nextFreeByte] = true;
	while (nextFreeByte < bytecode::NUM_BYTE_REGISTERS && byteRegs[nextFreeByte]) nextFreeByte++;

	return ret;
}

void compiler::gen::Frame::freew(bytecode::types::reg_t reg) {
	if (reg >= bytecode::reg::W0 && reg < bytecode::reg::B0) {
		int idx = reg - bytecode::reg::W0;
		assert(wordRegs[idx]);
		wordRegs[idx] = false;
	} else {
		throw std::domain_error("Invalid register to free");
	}
}

void compiler::gen::Frame::freeb(bytecode::types::reg_t reg) {
	if (reg >= bytecode::reg::B0 && reg < bytecode::reg::Count) {
		int idx = reg - bytecode::reg::B0;
		assert(byteRegs[idx]);
		byteRegs[idx] = false;
	} else {
		throw std::domain_error("Invalid register to free");
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Generation

template<typename T>
void compiler::gen::GenOut::write(T obj) {
	outputFile.write(reinterpret_cast<char*>(&obj), sizeof(T));
	byteCounter += sizeof(T);
}

int compiler::generateBytecode(std::iostream& outputFile, ast::Tree& astTree, CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler;
	using namespace compiler::gen;
	using namespace bytecode;
	
	GenOut output(outputFile);
	Frame globalFrame{};

	output.write<types::word_t>(4); // First instruction address (to be overwritten later)

	astTree.root->genBytecode(astTree, output, globalFrame, settings, stream);
	

	return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node generation

void compiler::ast::Node::genBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	throw std::logic_error("This node does not have bytecode generation");
}

void compiler::ast::Expr::genBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	bytecode::types::reg_t reg = genExprBytecode(astTree, output, frame, settings, stream);
	if (reg >= bytecode::reg::B0) {
		frame.freeb(reg);
	} else {
		frame.freew(reg);
	}
}

bytecode::types::reg_t compiler::ast::Expr::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	throw std::logic_error("This expr has unimplemented bytecode generation");
}

void compiler::ast::NodeGroup::genBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	if (type == NodeType::ROOT_GROUP) {
		for (Node* node : elems) {
			node->genBytecode(astTree, output, frame, settings, stream);
		}
	}
	// TODO: other group types
}

// ----

void compiler::ast::NodeMacro::genBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	// TODO
	// Need to restructure AST so Macro contains its target
}

bytecode::types::reg_t compiler::ast::NodeInt::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;
	types::reg_t reg = frame.allocw();
	output.write<types::opcode_t>(Opcode::R_MOV_W);
	output.write<types::reg_t>(reg);
	output.write<types::int_t>(val);
	return reg;
}

bytecode::types::reg_t compiler::ast::NodeFloat::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;
	types::reg_t reg = frame.allocw();
	output.write<types::opcode_t>(Opcode::R_MOV_W);
	output.write<types::reg_t>(reg);
	output.write<types::float_t>(val);
	return reg;
}

bytecode::types::reg_t compiler::ast::NodeChar::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;
	types::reg_t reg = frame.allocb();
	output.write<types::opcode_t>(Opcode::R_MOV_B);
	output.write<types::reg_t>(reg);
	output.write<types::char_t>(val);
	return reg;
}

bytecode::types::reg_t compiler::ast::NodeBool::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;
	types::reg_t reg = frame.allocb();
	output.write<types::opcode_t>(Opcode::R_MOV_B);
	output.write<types::reg_t>(reg);
	output.write<types::bool_t>(val);
	return reg;
}