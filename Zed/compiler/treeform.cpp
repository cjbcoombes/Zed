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

template<class N, class... Args>
N* compiler::ast::Tree::addNode(Args&&... args) {
	return add<N>(std::make_unique<N>(std::forward<Args>(args)...));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Node Constructors

compiler::ast::LiteralNode::LiteralNode(const Token* token) : Node(NodeType::LITERAL, token->loc), int_(0) {
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

compiler::ast::treeres_t compiler::ast::Match::formTree(Tree& tree, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG | compiler::FLAG_DEBUG_AST)) {
		stream << IO_DEBUG "Missing treeform implementation" IO_NORM "\n";
	}
	return { tree.add(std::make_unique<UnimplNode>("Generic match", loc)), 0 };
}

compiler::ast::treeres_t compiler::ast::TokenMatch::formTree(Tree& tree, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const {
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

compiler::ast::treeres_t compiler::ast::GroupMatch::formTree(Tree& tree, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const {
	// The parent match may decide to do something different with the group.

	compiler::ast::treeres_t tempRes;
	BlockNode* tempBlockNode;

	switch (type) {
		case MatchType::ROOT_GROUP:
		case MatchType::CURLY_GROUP:
			// block
			tempBlockNode = tree.add(std::make_unique<BlockNode>(loc));
			for (const Match* match : matches) {
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

static compiler::ast::treeres_t formArithBinop(const compiler::ast::FixedSizeMatch& match,
											   compiler::ast::Tree& tree,
											   compiler::CompilerStatus& status,
											   const compiler::CompilerSettings& settings,
											   std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	if (match.matches.size() != 3 || match.matches[1]->type != MatchType::TOKEN) {
		throw std::logic_error("Binary Arithmetic Match doesn't have exactly 3 children, where the second is a token");
	}
	treeres_t tempRes1 = match.matches[0]->formTree(tree, status, settings, stream);
	if (tempRes1.second) {
		return { nullptr, tempRes1.second };
	}
	treeres_t tempRes2 = match.matches[2]->formTree(tree, status, settings, stream);
	if (tempRes2.second) {
		return { nullptr, tempRes2.second };
	}

	ArithBinopNode::Type opType;
	switch (dynamic_cast<const TokenMatch*>(match.matches[1])->token->type) {
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
		status.addIssue(CompilerIssue::Type::INVALID_TYPE_ARITH_BINOP, match.loc);
	}

	return { tree.add(std::make_unique<ArithBinopNode>(opType, exprType, tempRes1.first, tempRes2.first, match.loc)), 0 };
}

static compiler::ast::treeres_t formMacro(const compiler::ast::FixedSizeMatch& match,
										  compiler::ast::Tree& tree,
										  compiler::CompilerStatus& status,
										  const compiler::CompilerSettings& settings,
										  std::ostream& stream) {
	using namespace compiler::ast;
	using namespace compiler;

	if (match.matches.size() != 3
		|| match.matches[1]->type != MatchType::TOKEN
		|| dynamic_cast<const TokenMatch*>(match.matches[1])->token->type != TokenType::IDENTIFIER) {
		throw std::logic_error("Macro Match doesn't have exactly three children, the second of which is an identifier");
	}

	treeres_t res = match.matches[2]->formTree(tree, status, settings, stream);
	if (res.second) {
		return { nullptr, res.second };
	}

	const std::string& str = *dynamic_cast<const TokenMatch*>(match.matches[1])->token->str;
	int strIndex = lookupString(str.c_str(), MacroNode::macroSpan);

	if (strIndex == -1) {
		status.addIssue(CompilerIssue::Type::INVALID_MACRO, match.matches[0]->loc, str);
		return { tree.add(std::make_unique<MacroNode>(MacroNode::Type::UNKNOWN, ExprType::primErrType, res.first, match.matches[0]->loc)), 0 };
	}

	MacroNode::Type type = static_cast<MacroNode::Type>(strIndex);
	ExprType exprType = ExprType::primNoType;

	switch (type) {
		case MacroNode::Type::HELLO_WORLD:
			// No type checking
			break;
		case MacroNode::Type::PRINT_I:
			if (!sameType(res.first->exprType, ExprType::primInt)) {
				status.addIssue(CompilerIssue::Type::INVALID_TYPE_MACRO, match.matches[2]->loc);
				exprType = ExprType::primErrType;
			} else {
				exprType = ExprType::primInt;
			}
			break;
		default:
			throw std::logic_error("Macro has impossible type");
	}

	return { tree.add(std::make_unique<MacroNode>(type, exprType, res.first, match.matches[0]->loc)), 0 };
}

compiler::ast::treeres_t compiler::ast::FixedSizeMatch::formTree(Tree& tree, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) const {
	treeres_t tempRes1;

	switch (type) {
		case MatchType::ARITH_BINOP:
			return formArithBinop(*this, tree, status, settings, stream);
			break;
		case MatchType::MACRO:
			return formMacro(*this, tree, status, settings, stream);
			break;
		default:
			UnimplNode* unimplNode = tree.add(std::make_unique<UnimplNode>("fixed size match with unhandled type", loc));
			for (const Match* match : matches) {
				tempRes1 = match->formTree(tree, status, settings, stream);
				if (tempRes1.second) {
					return { nullptr, tempRes1.second };
				}
				unimplNode->nodes.push_back(tempRes1.first);
			}
			return { unimplNode, 0 };
	}
}

int compiler::ast::formTree(ast::Tree& tree, const ast::MatchData& matchData, CompilerStatus& status, const CompilerSettings& settings, std::ostream& stream) {
	auto res = matchData.root->formTree(tree, status, settings, stream);
	tree.root = res.first;

	return res.second;
}