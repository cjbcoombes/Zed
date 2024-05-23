#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Types

const compiler::ast::ExprType compiler::ast::ExprType::primNoType{ PrimType::NONE };
const compiler::ast::ExprType compiler::ast::ExprType::primErrType{ PrimType::ERR };
const compiler::ast::ExprType compiler::ast::ExprType::primVoid{ PrimType::VOID };
const compiler::ast::ExprType compiler::ast::ExprType::primInt{ PrimType::INT };
const compiler::ast::ExprType compiler::ast::ExprType::primFloat{ PrimType::FLOAT };
const compiler::ast::ExprType compiler::ast::ExprType::primBool{ PrimType::BOOL };
const compiler::ast::ExprType compiler::ast::ExprType::primChar{ PrimType::CHAR };

bool compiler::ast::sameType(const ExprType& a, const ExprType& b) {
	return a.type == b.type;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Tree

template<class N>
N* compiler::ast::Tree::add(std::unique_ptr<N> node) {
	N* temp = node.get();
	nodes.emplace_back(std::move(node));
	return temp;
}

template<class N, class ...Args>
N* compiler::ast::Tree::make(Args&& ...args) {
	return add<N>(std::make_unique<N>(std::forward<Args>(args)...));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node Constructors

compiler::ast::LiteralNode::LiteralNode(Token* token) : Node(NodeType::LITERAL, token->loc), int_(0) {
	switch (token->type) {
		case TokenType::NUM_INT:
			int_ = token->int_;
			litType = Type::INT;
			exprType = ExprType::primInt;
			break;
		case TokenType::NUM_FLOAT:
			float_ = token->float_;
			litType = Type::FLOAT;
			exprType = ExprType::primFloat;
			break;
		case TokenType::CHAR:
			char_ = token->char_;
			litType = Type::CHAR;
			exprType = ExprType::primChar;
			break;
		case TokenType::TRUE:
			bool_ = true;
			litType = Type::BOOL;
			exprType = ExprType::primBool;
			break;
		case TokenType::FALSE:
			bool_ = false;
			litType = Type::BOOL;
			exprType = ExprType::primBool;
			break;
		default:
			throw std::logic_error("Literal Node constructed from Token not representing a literal");
			break;
	}
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Match Tree Formation

compiler::ast::treeres_t compiler::ast::Match::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG)) {
		stream << IO_DEBUG "Missing treeform implementation" IO_NORM "\n";
	}
	return { tree.add(std::make_unique<UnimplNode>("Generic match", loc)), 0 };
}

compiler::ast::treeres_t compiler::ast::TokenMatch::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	switch (token->type) {
		case TokenType::NUM_INT:
		case TokenType::NUM_FLOAT:
		case TokenType::CHAR:
		case TokenType::TRUE:
		case TokenType::FALSE:
			return { tree.add(std::make_unique<LiteralNode>(token)), 0 };
		default:
			return { tree.add(std::make_unique<TokenNode>(token)), 0 };
	}
}

compiler::ast::treeres_t compiler::ast::GroupMatch::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	// The parent match may decide to do something different with the group.

	compiler::ast::treeres_t tempRes;
	BlockNode* tempBlockNode;

	switch (type) {
		case MatchType::ROOT_GROUP:
		case MatchType::CURLY_GROUP:
			// block
			tempBlockNode = tree.add(std::make_unique<BlockNode>(loc));
			for (Match* match : matches) {
				tempRes = match->formTree(tree, status, settings, stream);
				if (tempRes.second) {
					return { nullptr, tempRes.second };
				}
				tempBlockNode->nodes.push_back(tempRes.first);
			}
			return { tempBlockNode, 0 };
		case MatchType::SQUARE_GROUP:
			return { tree.add(std::make_unique<UnimplNode>("Square group", loc)), 0 };
		case MatchType::PAREN_GROUP:
			if (matches.size() == 1) {
				return matches.front()->formTree(tree, status, settings, stream);
			} else {
				return { tree.add(std::make_unique<UnimplNode>("Paren group with multiple internal matches", loc)), 0 };
			}
			break;
		default:
			throw std::logic_error("GroupMatch has unexpected MatchType");
	}
}

compiler::ast::treeres_t formArithBinop(compiler::ast::FixedSizeMatch& match,
										compiler::ast::Tree& tree,
										compiler::CompilerStatus& status,
										compiler::CompilerSettings& settings,
										std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	treeres_t tempRes1;
	treeres_t tempRes2;

	if (match.matches.size() != 3 || match.matches[1]->type != MatchType::TOKEN) {
		throw std::logic_error("Binary Arithmetic Match doesn't have exactly 3 children, where the second is a token");
	}
	tempRes1 = match.matches[0]->formTree(tree, status, settings, stream);
	if (tempRes1.second) {
		return { nullptr, tempRes1.second };
	}
	tempRes2 = match.matches[2]->formTree(tree, status, settings, stream);
	if (tempRes2.second) {
		return { nullptr, tempRes2.second };
	}

	ArithBinopNode::Type opType;
	switch (dynamic_cast<TokenMatch*>(match.matches[1])->token->type) {
		case TokenType::PLUS:
			opType = ArithBinopNode::Type::ADD;
			break;
		case TokenType::DASH:
			opType = ArithBinopNode::Type::SUB;
			break;
		case TokenType::STAR:
			opType = ArithBinopNode::Type::MUL;
			break;
		case TokenType::SLASH:
			opType = ArithBinopNode::Type::DIV;
			break;
		default:
			throw std::logic_error("Binary Arithmetic Match has invalid arithmetic operation token");
	}

	ExprType exprType;
	if (sameType(tempRes1.first->exprType, ExprType::primInt) && sameType(tempRes2.first->exprType, ExprType::primInt)) {
		exprType = ExprType::primInt;
	} else if (sameType(tempRes1.first->exprType, ExprType::primFloat) && sameType(tempRes2.first->exprType, ExprType::primFloat)) {
		exprType = ExprType::primFloat;
	} else {
		exprType = ExprType::primErrType;
		status.addIssue(CompilerIssue(CompilerIssue::Type::INVALID_TYPE_ARITH_BINOP, match.loc));
	}

	return { tree.add(std::make_unique<ArithBinopNode>(opType, exprType, tempRes1.first, tempRes2.first, match.loc)), 0 };
}

compiler::ast::treeres_t formMacro(compiler::ast::FixedSizeMatch& match,
								   compiler::ast::Tree& tree,
								   compiler::CompilerStatus& status,
								   compiler::CompilerSettings& settings,
								   std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	const int size = match.matches.size();
	if ((size != 2 && size != 3) 
		|| match.matches[1]->type != MatchType::TOKEN 
		|| dynamic_cast<TokenMatch*>(match.matches[1])->token->type != TokenType::IDENTIFIER) {
		throw std::logic_error("Macro Match doesn't have exactly two or three children, the second of which is an identifier");
	}
	
	int expectedSize = -1;
	MacroNode::Type type = MacroNode::Type::HELLO_WORLD;

	std::string& str = *dynamic_cast<TokenMatch*>(match.matches[1])->token->str;
	
	// TODO: put these in arrays and automate instead of manually doing each
	if (str == "printi") {
		expectedSize = 3;
		type = MacroNode::Type::PRINT_I;
	}

	if (str == "hello_world") {
		expectedSize = 2;
		type = MacroNode::Type::HELLO_WORLD;
	}

	if (expectedSize == -1) {
		// TODO: change patterns to be more allowing, and make this a user error rather than a compiler error
		throw std::logic_error("Macro Match doesn't match any known macro");
	}

	std::optional<MacroNode::Type> macroType = std::nullopt;

}

compiler::ast::treeres_t compiler::ast::FixedSizeMatch::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	treeres_t tempRes1;

	switch (type) {
		case MatchType::ARITH_BINOP:
			return formArithBinop(*this, tree, status, settings, stream);
			break;
		default:
			UnimplNode* unimplNode = tree.add(std::make_unique<UnimplNode>("fixed size match with unhandled type", loc));
			for (Match* match : matches) {
				tempRes1 = match->formTree(tree, status, settings, stream);
				if (tempRes1.second) {
					return { nullptr, tempRes1.second };
				}
				unimplNode->nodes.push_back(tempRes1.first);
			}
			return { unimplNode, 0 };
	}
}

int compiler::ast::formTree(ast::Tree& tree, ast::MatchData& matchData, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	auto res = matchData.root->formTree(tree, status, settings, stream);
	tree.root = res.first;

	return res.second;
}