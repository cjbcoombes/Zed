#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node/Tree Printing

#define TREE_BRANCH_MID "+-- "
#define TREE_BRANCH_END "\\-- "
#define TREE_SPACE "    "
#define TREE_PASS "|   "
#define TYPE_CONN " -> "

void compiler::ast::ExprType::printSimple(std::ostream& stream) {
	switch (type) {
		case PrimType::NONE:
			stream << "*";
			break;
		case PrimType::VOID:
			stream << IO_FMT_KEYWORD("void");
			break;
		case PrimType::INT:
			stream << IO_FMT_KEYWORD("int");
			break;
		case PrimType::FLOAT:
			stream << IO_FMT_KEYWORD("float");
			break;
		case PrimType::CHAR:
			stream << IO_FMT_KEYWORD("char");
			break;
		case PrimType::BOOL:
			stream << IO_FMT_KEYWORD("bool");
			break;
		default:
			stream << "Complex Type"; // TODO: make good
	}
}

void compiler::ast::Node::print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	printSimple(tokenData, stream);
	printType(stream);
	stream << '\n';
}

void compiler::ast::Node::printSimple(TokenData& tokenData, std::ostream& stream) {
	stream << "??? Node ???";
}

void compiler::ast::Node::printType(std::ostream& stream) {
	if (!sameType(exprType, ExprType::primNoType)) {
		stream << TYPE_CONN;
		exprType.printSimple(stream);
	}
}

void compiler::ast::UnimplNode::print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Unimplemented ( " << msg << " )\n";

	for (auto i = nodes.cbegin(); i != nodes.cend(); i++) {
		(*i)->print(tokenData, stream, indent + (last ? TREE_SPACE : TREE_PASS), std::next(i) == nodes.end());
	}
}

void compiler::ast::TokenNode::printSimple(TokenData& tokenData, std::ostream& stream) {
	stream << "Token ( ";
	compiler::printToken(*token, tokenData, stream);
	stream << " )";
}

void compiler::ast::LiteralNode::printSimple(TokenData& tokenData, std::ostream& stream) {
	stream << "Lit ( ";
	switch (litType) {
		case Type::INT:
			stream << IO_FMT_INT(int_);
			break;
		case Type::FLOAT:
			stream << IO_FMT_FLOAT(float_);
			break;
		case Type::CHAR:
			stream << IO_FMT_CHAR(char_);
			break;
		case Type::BOOL:
			stream << IO_FMT_KEYWORD((bool_ ? "true" : "false"));
			break;
	}
	stream << " )";
}

void compiler::ast::BlockNode::print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Block";
	printType(stream);
	stream << '\n';

	for (auto i = nodes.cbegin(); i != nodes.cend(); i++) {
		(*i)->print(tokenData, stream, indent + (last ? TREE_SPACE : TREE_PASS), std::next(i) == nodes.end());
	}
}

void compiler::ast::ArithBinopNode::print(TokenData& tokenData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Arith Binop ( ";
	if (opType == Type::ADD) stream << "+";
	else if (opType == Type::SUB) stream << "-";
	else if (opType == Type::MUL) stream << "*";
	else if (opType == Type::DIV) stream << "/";
	stream << " )";
	printType(stream);
	stream << '\n';

	left->print(tokenData, stream, indent + (last ? TREE_SPACE : TREE_PASS), false);
	right->print(tokenData, stream, indent + (last ? TREE_SPACE : TREE_PASS), true);
}

void compiler::ast::Tree::print(TokenData& tokenData, std::ostream& stream) {
	if (root) root->print(tokenData, stream, "", true);
}