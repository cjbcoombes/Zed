#include "nodes.h"
#include "../utils/io_utils.h"
#include "tokenizer.h"
#include "treeform.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node/Tree Printing

#define TREE_BRANCH_MID "+-- "
#define TREE_BRANCH_END "\\-- "
#define TREE_SPACE "    "
#define TREE_PASS "|   "
#define TYPE_CONN " -> "

void compiler::ast::TypeData::printType(const Type* const t, std::ostream& stream) const {
	if (t == nullptr) {
		stream << "*";
		return;
	}

	if (t->name.has_value()) {
		stream << IO_FMT_ID(**(t->name)) << " : ";
	}

	if (t->primType.has_value()) {
		PrimType pt = *t->primType;
		switch (pt) {
		case PrimType::ERR:
			stream << IO_FMT_ERR("err-type");
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
		case PrimType::BOOL:
			stream << IO_FMT_KEYWORD("bool");
			break;
		case PrimType::CHAR:
			stream << IO_FMT_KEYWORD("char");
			break;
		}
	} else {
		const auto sz = t->subtypes.size();
		if (sz == 0) {
			stream << "[ type with no internals ]";
		} else {
			stream << "[ ";
			for (auto tt = t->subtypes.cbegin(), 
				 end = t->subtypes.cend(), 
				 prev = std::prev(end); tt != end; ++tt) {
				printType(*tt, stream);
				if (tt != prev) {
					stream << ", ";
				}
			}
			stream << " ]";
		}
	}

	if (t->returnType != nullptr) {
		stream << " -> ";
		printType(t->returnType, stream);
	}
}

void compiler::ast::TypeData::printType(const ExprType type, std::ostream& stream) const {
	printType(type.type, stream);
}

void compiler::ast::Node::print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	printSimple(typeData, stream);
	printType(typeData, stream);
	stream << '\n';
}

void compiler::ast::Node::printSimple(const TypeData& typeData, std::ostream& stream) const {
	stream << "??? Node ???";
}

void compiler::ast::Node::printType(const TypeData& typeData, std::ostream& stream) const {
	if (!typeData.isNoneType(exprType)) {
		stream << TYPE_CONN;
		typeData.printType(exprType, stream);
	}
}

void compiler::ast::UnimplNode::print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Unimplemented ( " << msg << " )\n";

	for (auto i = nodes.cbegin(); i != nodes.cend(); i++) {
		(*i)->print(typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), std::next(i) == nodes.end());
	}
}

void compiler::ast::TokenNode::printSimple(const TypeData& typeData, std::ostream& stream) const {
	stream << "Token ( ";
	compiler::tokens::printToken(*token, stream);
	stream << " )";
}

void compiler::ast::LiteralNode::printSimple(const TypeData& typeData, std::ostream& stream) const {
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

void compiler::ast::BlockNode::print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Block";
	printType(typeData, stream);
	stream << '\n';

	for (auto i = nodes.cbegin(); i != nodes.cend(); i++) {
		(*i)->print(typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), std::next(i) == nodes.end());
	}
}

void compiler::ast::ArithBinopNode::print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Arith Binop ( ";
	if (opType == Type::ADD) stream << "+";
	else if (opType == Type::SUB) stream << "-";
	else if (opType == Type::MUL) stream << "*";
	else if (opType == Type::DIV) stream << "/";
	stream << " )";
	printType(typeData, stream);
	stream << '\n';

	left->print(typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), false);
	right->print(typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), true);
}

void compiler::ast::MacroNode::print(const TypeData& typeData, std::ostream& stream, std::string&& indent, bool last) const {
	stream << indent;
	if (last) stream << TREE_BRANCH_END;
	else stream << TREE_BRANCH_MID;
	stream << "Macro ( #";
	stream << macroStrings[static_cast<int>(macroType)];
	stream << " )";
	printType(typeData, stream);
	stream << '\n';

	arg->print(typeData, stream, indent + (last ? TREE_SPACE : TREE_PASS), true);
}

void compiler::ast::TypeNode::printSimple(const TypeData& typeData, std::ostream& stream) const {
	stream << "Type ( ";
	typeData.printType(repType, stream);
	stream << " )";
}

void compiler::ast::Tree::print(std::ostream& stream) const {
	if (root) root->print(typeData, stream, "", true);
	else stream << "[tree root not set]";
}