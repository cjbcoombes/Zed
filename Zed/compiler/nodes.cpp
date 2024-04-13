#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node/Tree Printing

#define TREE_BRANCH_MID "+-- "
#define TREE_BRANCH_END "\\-- "
#define TREE_SPACE "    "
#define TREE_PASS "|   "

//#define TREE_BRANCH_MID u8"\u2523\u2501 "
//#define TREE_BRANCH_END u8"\u2517\u2501 "
//#define TREE_SPACE "   "
//#define TREE_PASS u8"\u2503  "

void compiler::ast::Node::print(TokenData& tokenData, TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	printSimple(tokenData, typeData, stream);
}

void compiler::ast::Node::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "??? Node ??? \n";
}

void compiler::ast::Node::printToken(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	if (token) {
		compiler::printToken(*token, tokenData, stream);
	} else {
		stream << "NO TOKEN";
	}
}

void compiler::ast::NodeToken::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "Token ( ";
	printToken(tokenData, typeData, stream);
	stream << " )\n";
}

void compiler::ast::NodeMacro::print(TokenData& tokenData, TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Macro ( #" << IO_FMT_ID(macroStrings[static_cast<int>(macroType)]) << " )\n";

	target->print(tokenData, typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), true);
}

void compiler::ast::NodeTypeSpec::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "Type ( ";
	// Note: indices much match TypeData
	switch (exprType.index) {
		case -1:
			stream << IO_FMT_KEYWORD("undefined");
			break;
		case 0:
			stream << IO_FMT_KEYWORD("void");
			break;
		case 1:
			stream << IO_FMT_KEYWORD("int");
			break;
		case 2:
			stream << IO_FMT_KEYWORD("float");
			break;
		case 3:
			stream << IO_FMT_KEYWORD("char");
			break;
		case 4:
			stream << IO_FMT_KEYWORD("bool");
			break;
		default:
			stream << "Complex Type"; // TODO: make good
	}
	stream << " )\n";
}

void compiler::ast::NodeFunDef::print(TokenData& tokenData, TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Fun ( " << tokenData.strList[strIndex] << " )\n";

	body->print(tokenData, typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), true);
}

void compiler::ast::NodeInt::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "Int ( " << IO_FMT_INT(val) << " )\n";
}

void compiler::ast::NodeFloat::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "Float ( " << IO_FMT_FLOAT(val) << " )\n";
}

void compiler::ast::NodeChar::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "Char ( " << IO_FMT_CHAR(val) << " )\n";
}

void compiler::ast::NodeString::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "String ( " << IO_FMT_STRING(tokenData.strList[strIndex]) << " )\n";
}

void compiler::ast::NodeBool::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "Bool ( " << IO_FMT_KEYWORD((val ? "true" : "false")) << " )\n";
}

void compiler::ast::NodeIdentifier::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "Identifier ( " << tokenData.strList[strIndex] << " )\n";
}

void compiler::ast::NodeGroup::print(TokenData& tokenData, TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Group ";
	if (type == NodeType::ROOT_GROUP) stream << "--root--";
	else if (type == NodeType::PAREN_GROUP) stream << "()";
	else if (type == NodeType::SQUARE_GROUP) stream << "[]";
	else if (type == NodeType::CURLY_GROUP) stream << "{}";
	else if (type == NodeType::ANGLE_GROUP) stream << "<>";
	else stream << "??";
	stream << '\n';

	for (auto i = elems.cbegin(); i != elems.cend(); i++) {
		(*i)->print(tokenData, typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), std::next(i) == elems.end());
	}
}

void compiler::ast::NodeArithBinop::print(TokenData& tokenData, TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) {

	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Arith Binop ( ";
	if (opType == OpType::ADD) stream << "+";
	else if (opType == OpType::SUB) stream << "-";
	else if (opType == OpType::MUL) stream << "*";
	else if (opType == OpType::DIV) stream << "/";
	else stream << "??";
	stream << " )\n";

	left->print(tokenData, typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), false);
	right->print(tokenData, typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), true);
}

void compiler::ast::Tree::print(std::ostream& stream) {
	if (root) root->print(tokenData, typeData, stream, "", true);
}