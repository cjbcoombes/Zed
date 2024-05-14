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

void compiler::ast::Type::printSimple(std::ostream& stream) {
	// Note: indices much match TypeData
	switch (index) {
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
}

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
	exprType.printSimple(stream);
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

void compiler::ast::NodeDeclaration::printSimple(TokenData& tokenData, TypeData& typeData, std::ostream& stream) {
	stream << "Declaration ( "; 
	exprType.printSimple(stream);
	stream << " . " << tokenData.strList[strIndex] << " )\n";
}

void compiler::ast::NodeAssignment::print(TokenData& tokenData, TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) {

	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Assignment ( " << tokenData.strList[left->strIndex] << " )\n";
	right->print(tokenData, typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), true);
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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Scopes

void compiler::ast::Scope::push() {
	scopes.push(++scopeIndex);
}

void compiler::ast::Scope::pop() {
	scopes.pop();
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node/Tree formedness

void compiler::ast::Node::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	// nothing!
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG)) {
		stream << IO_DEBUG "raw node form check was called" IO_NORM "\n";
	}
}

void compiler::ast::NodeGroup::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	if (type == NodeType::CURLY_GROUP || type == NodeType::ROOT_GROUP) {
		scope.push();
		for (Node* elem : elems) {
			elem->checkForm(astTree, scope, settings, stream);
		}
		scope.pop();
	} else {
		throw std::logic_error("Check form not implemented for other group types yet");
	}
}

void compiler::ast::NodeToken::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	throw CompilerException(CompilerException::ErrorType::BAD_FORM, token->line, token->column, "invalid token in tree");
}

void compiler::ast::NodeMacro::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	target->checkForm(astTree, scope, settings, stream);
}

void compiler::ast::NodeTypeSpec::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	// nothing!
}

void compiler::ast::NodeFunDef::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	Scope newScope{};
	// TODO: add to scope
	body->checkForm(astTree, newScope, settings, stream);
}

void compiler::ast::NodeInt::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	// nothing!
}

void compiler::ast::NodeFloat::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	// nothing!
}

void compiler::ast::NodeChar::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	// nothing!
}

void compiler::ast::NodeBool::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	// nothing!
}

void compiler::ast::NodeString::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	// nothing!
}

void compiler::ast::NodeIdentifier::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	// nothing!
}

void compiler::ast::NodeDeclaration::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	// nothing!
}

void compiler::ast::NodeAssignment::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	left->checkForm(astTree, scope, settings, stream);
	right->checkForm(astTree, scope, settings, stream);
	
	if (!TypeData::sameCompatible(left->exprType, right->exprType)) {
		throw CompilerException(CompilerException::ErrorType::MISMATCH_TYPE, token->line, token->column, "mismatched types in assignment");
	}
}

void compiler::ast::NodeArithBinop::checkForm(Tree& astTree, Scope& scope, CompilerSettings& settings, std::ostream& stream) {
	left->checkForm(astTree, scope, settings, stream);
	right->checkForm(astTree, scope, settings, stream);

	if (!(TypeData::sameExact(left->exprType, TypeData::typeInt) ||
		  TypeData::sameExact(left->exprType, TypeData::typeFloat))) {
		throw CompilerException(CompilerException::ErrorType::BAD_TYPE, left->token->line, left->token->column, "invalid type on left of arithmetic binop");
	}

	if (!(TypeData::sameExact(right->exprType, TypeData::typeInt) ||
		  TypeData::sameExact(right->exprType, TypeData::typeFloat))) {
		throw CompilerException(CompilerException::ErrorType::BAD_TYPE, right->token->line, right->token->column, "invalid type on right of arithmetic binop");
	}

	if (!TypeData::sameCompatible(left->exprType, right->exprType)) {
		throw CompilerException(CompilerException::ErrorType::MISMATCH_TYPE, token->line, token->column, "mismatched types of arithmetic binop");
	}

	exprType = left->exprType;
}