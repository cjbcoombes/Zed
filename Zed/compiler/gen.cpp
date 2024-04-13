#include "compiler.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Stack Frame Management

compiler::gen::Frame::Frame() : bytes(), wordRegs(), byteRegs() {
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

template<typename T>
void compiler::gen::Frame::write(T obj) {
	char* p = reinterpret_cast<char*>(&obj);
	for (int i = 0; i < sizeof(T); i++) {
		bytes.push_back(*p);
		p++;
	}
}

compiler::gen::Frame& compiler::gen::GenOut::addFrame() {
	std::unique_ptr<Frame> frame = std::make_unique<Frame>();
	Frame& temp = *frame.get();
	frames.push_back(std::move(frame));
	return temp;
}

int compiler::generateBytecode(std::iostream& outputFile, ast::Tree& astTree, CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler;
	using namespace compiler::gen;
	using namespace bytecode;
	
	GenOut output;
	Frame& globalFrame = output.addFrame();

	globalFrame.write<types::word_t>(4); // First instruction address (to be overwritten later)

	astTree.root->genBytecode(astTree, output, globalFrame, settings, stream);
	
	for (char byte : globalFrame.bytes) {
		outputFile.write(&byte, 1);
	}

	return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node generation

void compiler::ast::Node::genBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	throw std::logic_error("This node does not have bytecode generation");
}

void compiler::ast::NodeFunDef::genBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace compiler;
	using namespace compiler::gen;
	using namespace bytecode;

	Frame& newFrame = output.addFrame();
	body->genBytecode(astTree, output, newFrame, settings, stream);
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
	if (type == NodeType::ROOT_GROUP || type == NodeType::CURLY_GROUP) {
		for (Node* node : elems) {
			node->genBytecode(astTree, output, frame, settings, stream);
		}
	}
	// TODO: other group types
}

// ----

void compiler::ast::NodeMacro::genBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;

	types::reg_t reg;
	
	switch (macroType) {
		case MacroType::PRINTI:
			reg = target->genExprBytecode(astTree, output, frame, settings, stream);
			frame.write<types::opcode_t>(Opcode::R_PRNT_I);
			frame.write<types::reg_t>(reg);
			frame.freew(reg);
			break;
		case MacroType::PRINTF:
			reg = target->genExprBytecode(astTree, output, frame, settings, stream);
			frame.write<types::opcode_t>(Opcode::R_PRNT_F);
			frame.write<types::reg_t>(reg);
			frame.freew(reg);
			break;
	}
}

bytecode::types::reg_t compiler::ast::NodeInt::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;
	types::reg_t reg = frame.allocw();
	frame.write<types::opcode_t>(Opcode::MOV_W);
	frame.write<types::reg_t>(reg);
	frame.write<types::int_t>(val);
	return reg;
}

bytecode::types::reg_t compiler::ast::NodeFloat::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;
	types::reg_t reg = frame.allocw();
	frame.write<types::opcode_t>(Opcode::MOV_W);
	frame.write<types::reg_t>(reg);
	frame.write<types::float_t>(val);
	return reg;
}

bytecode::types::reg_t compiler::ast::NodeChar::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;
	types::reg_t reg = frame.allocb();
	frame.write<types::opcode_t>(Opcode::MOV_B);
	frame.write<types::reg_t>(reg);
	frame.write<types::char_t>(val);
	return reg;
}

bytecode::types::reg_t compiler::ast::NodeBool::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;
	types::reg_t reg = frame.allocb();
	frame.write<types::opcode_t>(Opcode::MOV_B);
	frame.write<types::reg_t>(reg);
	frame.write<types::bool_t>(val);
	return reg;
}

bytecode::types::reg_t compiler::ast::NodeArithBinop::genExprBytecode(Tree& astTree, gen::GenOut& output, gen::Frame& frame, CompilerSettings& settings, std::ostream& stream) {
	using namespace bytecode;
	static constexpr types::opcode_t opcodes[][4] = {
		{ Opcode::I_ADD, Opcode::I_SUB, Opcode::I_MUL, Opcode::I_DIV }, // ints
		{ Opcode::F_ADD, Opcode::F_SUB, Opcode::F_MUL, Opcode::F_DIV } // floats
	};

	int i = TypeData::sameExact(left->exprType, TypeData::typeInt) ? 0 :
		TypeData::sameExact(left->exprType, TypeData::typeFloat) ? 1 :
		-1;

	int j = opType == OpType::ADD ? 0 :
		opType == OpType::SUB ? 1 :
		opType == OpType::MUL ? 2 :
		opType == OpType::DIV ? 3 : -1; // -1 should be impossible because enum

	if (i < 0 || j < 0 || !TypeData::sameExact(left->exprType, right->exprType)) {
		throw std::domain_error("Invalid type for arith binop");
	}

	types::reg_t reg1 = left->genExprBytecode(astTree, output, frame, settings, stream);
	types::reg_t reg2 = right->genExprBytecode(astTree, output, frame, settings, stream);
	frame.write<types::opcode_t>(opcodes[i][j]);
	frame.write<types::reg_t>(reg1);
	frame.write<types::reg_t>(reg1);
	frame.write<types::reg_t>(reg2);

	if (i <= 1) {
		frame.freew(reg2);
	} else {
		frame.freeb(reg2);
	}

	return reg1;
}