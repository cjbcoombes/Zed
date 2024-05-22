#include "compiler.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Types

const compiler::ast::ExprType compiler::ast::ExprType::primNoType{ PrimType::NONE };
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

compiler::ast::LiteralNode::LiteralNode(Token* token) : Node(NodeType::LITERAL, token->line, token->column), int_(0) {
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

std::pair<compiler::ast::Node*, int> compiler::ast::Match::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	if (settings.flags.hasFlags(Flags::FLAG_DEBUG)) {
		stream << IO_DEBUG "Missing treeform implementation" IO_NORM "\n";
	}
	return { tree.add(std::make_unique<UnimplNode>("Generic match", line, column)), 0 };
}

std::pair<compiler::ast::Node*, int> compiler::ast::TokenMatch::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
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

std::pair<compiler::ast::Node*, int> compiler::ast::GroupMatch::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	// The parent match may decide to do something different with the group.

	std::pair<compiler::ast::Node*, int> tempRes;
	BlockNode* tempBlockNode;

	switch (type) {
		case MatchType::ROOT_GROUP:
		case MatchType::CURLY_GROUP:
			// block
			tempBlockNode = tree.add(std::make_unique<BlockNode>(line, column));
			for (Match* match : matches) {
				tempRes = match->formTree(tree, status, settings, stream);
				if (tempRes.second) {
					return { nullptr, tempRes.second };
				}
				tempBlockNode->nodes.push_back(tempRes.first);
			}
			return { tempBlockNode, 0 };
		case MatchType::SQUARE_GROUP:
			return { tree.add(std::make_unique<UnimplNode>("Square group", line, column)), 0 };
		case MatchType::PAREN_GROUP:
			if (matches.size() == 1) {
				return matches.front()->formTree(tree, status, settings, stream);
			} else {
				return { tree.add(std::make_unique<UnimplNode>("Paren group with multiple internal matches", line, column)), 0 };
			}
			break;
		default:
			throw std::logic_error("GroupMatch has unexpected MatchType");
	}
}

std::pair<compiler::ast::Node*, int> compiler::ast::FixedSizeMatch::formTree(Tree& tree, CompilerStatus& status, CompilerSettings& settings, std::ostream& stream) {
	std::pair<compiler::ast::Node*, int> tempRes1;
	std::pair<compiler::ast::Node*, int> tempRes2;

	switch (type) {
		case MatchType::ARITH_BINOP:
			if (matches.size() != 3 || matches[1]->type != MatchType::TOKEN) {
				throw std::logic_error("Binary Arithmetic Match doesn't have exactly 3 children, where the second is a token");
			}
			tempRes1 = matches[0]->formTree(tree, status, settings, stream);
			if (tempRes1.second) {
				return { nullptr, tempRes1.second };
			}
			tempRes2 = matches[2]->formTree(tree, status, settings, stream);
			if (tempRes2.second) {
				return { nullptr, tempRes2.second };
			}
			ArithBinopNode::Type opType;
			switch (dynamic_cast<TokenMatch*>(matches[1])->token->type) {
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

			return { tree.add(std::make_unique<ArithBinopNode>(opType, tempRes1.first, tempRes2.first, line, column)), 0 };

		default:
			UnimplNode* unimplNode = tree.add(std::make_unique<UnimplNode>("fixed size match with unhandled type", line, column));
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